#ifndef PS_CORE_PARSER_H
#define PS_CORE_PARSER_H

#include "types.h"
#include "runtime.h"
#include "conds.h"
#include "parser/adt.h"
#include "parser/functor.h"
#include "parser/bind.h"
#include "interpreter.h"

namespace ps
{
namespace core
{

// New design

// Interface

template <typename A>
ParserResult<A> parse(
        const ParserL<A>& pst,
        const std::string& s,
        Pos from = 0)
{
    ParserRuntime runtime (s, State{});
    return runParser(runtime, pst, from);
}

template <typename A>
ParserResult<A> parse_with_runtime(
    ParserRuntime &runtime,
    const ParserL<A> &pst,
    Pos from = 0)
{
  return runParser(runtime, pst, from);
}

// Parsers

ParserL<Char> parseSymbolCond(
        const std::string& name,
        const std::function<bool(char)>& cond)
{
  std::function<bool(char)> cond_copy = cond;

  std::function<bool(Any)> condAny = [=](const Any& any)  {
      char ch = std::any_cast<char>(any);
      return cond_copy(ch);
  };

    return make_free(ParseSymbolCond<ParserL<Char>>{
                      name,
                      condAny,
                      [](const Any& any)  {
                          char ch = std::any_cast<char>(any);
                          {
                              return make_pure(ch);
                          }
                      }
                });
}

ParserL<std::string> parseLit(const std::string& s)
{
    return make_free(ParseLit<ParserL<std::string>>{
                      s,
                      [](const std::string& resS) {
                          return make_pure(resS);
                      }
                });
}

// TODO: satisfy
// template <typename A>
// ParserL<A> satisfy(const std::function<bool(A)>& cond)
// {
//     return
// }


const ParserL<Char> digit = parseSymbolCond("digit", isDigit);
const ParserL<Char> upper = parseSymbolCond("upper", isUpper);
const ParserL<Char> lower = parseSymbolCond("lower", isLower);
const ParserL<Char> alpha = parseSymbolCond("alpha", isAlpha);
const ParserL<Char> alphanum = parseSymbolCond("alphanum", isAlphanum);
const ParserL<Char> space = parseSymbolCond("space", isSpace);
const ParserL<Char> anyChar = parseSymbolCond("any char", [](char) { return true; });
const ParserL<Char> eol = parseSymbolCond("eol", isEol);
const ParserL<Char> cr = parseSymbolCond("cr", isCr);

template <typename A>
ParserL<Many<A>> many(const ParserL<A>& item)
{
  ParserL<A> itemCopy = item;

  auto pCopy = std::make_shared<ParserL<Any>>(
      fmap<A, Any>([](const A &a)
                   { return a; }, itemCopy));

  return make_free(ParseMany<ParserL<Many<A>>>{
      pCopy,
      [](const std::list<Any> &resList)
      {
        Many<A> res;
        for (const Any &any : resList)
        {
          A a = std::any_cast<A>(any);
          res.push_back(a);
        }
        return make_pure(res);
      }});
}

// seq: run two parsers in sequence and return the result of the second one
template <typename A, typename B>
ParserL<B> seq(const ParserL<A>& p, const ParserL<B>& q)
{
    return bind<A, B>(p, [=](const A&) { return q; });
}

// TODO: test it
template <typename A>
ParserL<Many<A>> bothSequence(const ParserL<A>& fst, const ParserL<A>& snd)
{
    return bind<A, Many<A>>(fst, [=](const A& a) {
        return fmap<A, Many<A>>([=](const A& b) {
            Many<A> res;
            res.push_back(a);
            res.push_back(b);
            return res;
        }, snd);
    });
}

// TODO: test it
template <typename A, typename B>
ParserL<std::pair<A, B>> both(const ParserL<A>& fst, const ParserL<B>& snd)
{
    return bind<A, std::pair<A, B>>(fst, [=](const A& a) {
        return fmap<B, std::pair<A, B>>([=](const B& b) {
            return std::make_pair(a, b);
        }, snd);
    });
}

template <typename A, typename B>
ParserL<A> left(const ParserL<A>& p, const ParserL<B>& q)
{
    return bind<A, A>(p, [=](const A& a) {
        return fmap<B, A>([=](const B&) { return a; }, q);
    });
}

template <typename A, typename B>
ParserL<B> right(const ParserL<A>& p, const ParserL<B>& q)
{
    return bind<A, B>(p, [=](const A&) { return q; });
}

template <typename A>
ParserL<Many<A>> many1(const ParserL<A>& p)
{
    ParserL<Many<A>> manyP = many<A>(p);

    return bind<A, Many<A>>(p, [=](const A& a) {
        return fmap<Many<A>, Many<A>>([=](Many<A> rest) {
            rest.push_front(a);
            return rest;
        }, manyP);
    });
}

template <typename A, typename S>
ParserL<Many<A>> sepBy1(const ParserL<A>& item, const ParserL<S>& sep)
{
    // sepThenItem = sep *> item
    ParserL<A> sepThenItem = bind<S, A>(sep, [=](const S&) { return item; });

    ParserL<A> itemCopy = item;
    ParserL<Many<A>> restParser = many<A>(sepThenItem);

    return bind<A, Many<A>>(item, [=](const A& first) {
        return fmap<Many<A>, Many<A>>([=](Many<A> rest) {
            rest.push_front(first);
            return rest;
        }, restParser);
    });
}

template <typename O, typename A, typename C>
ParserL<A> between(const ParserL<O>& open, const ParserL<A>& content, const ParserL<C>& close)
{
    return bind<O, A>(open, [=](const O&) {
        return bind<A, A>(content, [=](const A& a) {
            return bind<C, A>(close, [=](const C&) {
                return make_pure(a);
            });
        });
    });
}

template <typename A>
ParserL<Many<A>> count(size_t n, const ParserL<A>& p)
{
    if (n == 0)
    {
        return make_pure(Many<A>{});
    }

    // create a copy for passing to many/count recursively
    ParserL<A> pCopy = p;

    return bind<A, Many<A>>(p, [=](const A& a) {
        return fmap<Many<A>, Many<A>>([=](Many<A> rest) {
            rest.push_front(a);
            return rest;
        }, count(n - 1, pCopy));
    });
}

template <typename A, typename B, typename R>
ParserL<R> liftA2(const std::function<R(A, B)>& f, const ParserL<A>& pa, const ParserL<B>& pb)
{
    return bind<A, R>(pa, [=](const A& a) {
        return fmap<B, R>([=](const B& b) {
            return f(a, b);
        }, pb);
    });
}

// This combinator needs some investigation.
// template <typename A, typename R>
// ParserL<R> ap(const ParserL<std::function<R(A)>>& pf, const ParserL<A>& pa)
// {
//     return bind<std::function<R(A)>, R>(pf, [=](const std::function<R(A)>& f) {
//         return fmap<A, R>(f, pa);
//     });
// }

template <typename A>
ParserL<Unit> discard(const ParserL<A>& p)
{
    return fmap<A, Unit>([=](const A&) { return unit; }, p);
}


/// Some specific combinators
// TODO: test it
ParserL<std::string> manyCharsToString(
  const ParserL<Many<Char>> &manyCharsParser)
{
  return fmap<Many<Char>, std::string>(
      [](const Many<Char>& chars) {
          return std::string(chars.begin(), chars.end());
      },
      manyCharsParser);
}



} // namespace core
} // namespace ps

#endif // PS_CORE_PARSER_H
