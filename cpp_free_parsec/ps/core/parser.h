#ifndef PS_CORE_PARSER_H
#define PS_CORE_PARSER_H

#include "types.h"
#include "runtime.h"
#include "conds.h"
#include "parser/adt.h"
#include "parser/functor.h"
#include "parser/bind.h"
#include "interpreter.h"

#include <tuple>
#include <type_traits>
#include <utility>

namespace ps
{
namespace core
{

// New design

// Interface

template <typename A>
ParserResult<A> parse(
        const Parser<A>& pst,
        const std::string& s,
        Pos from = 0)
{
    ParserRuntime runtime (s, State{});
    return runParser(runtime, pst, from);
}

template <typename A>
ParserResult<A> parseWithRuntime(
    ParserRuntime &runtime,
    const Parser<A> &pst,
    Pos from = 0)
{
  return runParser(runtime, pst, from);
}

// Parsers

template <typename Dummy = int>
Parser<Char> parseSymbolCond(
        const std::string& name,
        const std::function<bool(char)>& cond)
{
  std::function<bool(char)> cond_copy = cond;

  std::function<bool(Any)> condAny = [=](const Any& any)  {
      char ch = std::any_cast<char>(any);
      return cond_copy(ch);
  };

    return makeFree(ParseSymbolCond<Parser<Char>>{
                      name,
                      condAny,
                      [](const Any& any)  {
                          char ch = std::any_cast<char>(any);
                          {
                              return makePure(ch);
                          }
                      }
                });
}

template <typename Dummy = int>
Parser<Char> parseChar(char expected)
{
    return parseSymbolCond("", [=](char ch) { return ch == expected; });
}

template <typename Dummy = int>
Parser<std::string> parseLit(const std::string& s)
{
    return makeFree(ParseLit<Parser<std::string>>{
                      s,
                      [](const std::string& resS) {
                          return makePure(resS);
                      }
                });
}

// TODO: satisfy
// template <typename A>
// Parser<A> satisfy(const std::function<bool(A)>& cond)
// {
//     return
// }


const Parser<Char> digit = parseSymbolCond("digit", isDigit);
const Parser<Char> upper = parseSymbolCond("upper", isUpper);
const Parser<Char> lower = parseSymbolCond("lower", isLower);
const Parser<Char> alpha = parseSymbolCond("alpha", isAlpha);
const Parser<Char> alphanum = parseSymbolCond("alphanum", isAlphanum);
const Parser<Char> space = parseSymbolCond("space", isSpace);
const Parser<Char> anyChar = parseSymbolCond("any char", [](char) { return true; });
const Parser<Char> eol = parseSymbolCond("eol", isEol);
const Parser<Char> cr = parseSymbolCond("cr", isCr);
const Parser<Char> comma = parseSymbolCond("comma", [](char ch)
                                             { return ch == ','; });

template <typename A>
Parser<Many<A>> many(const Parser<A>& item)
{
  Parser<A> itemCopy = item;

  auto pCopy = std::make_shared<Parser<Any>>(
      fmap<A, Any>([](const A &a)
                   { return a; }, itemCopy));

  return makeFree(ParseMany<Parser<Many<A>>>{
      pCopy,
      [](const std::list<Any> &resList)
      {
        Many<A> res;
        for (const Any &any : resList)
        {
          A a = std::any_cast<A>(any);
          res.push_back(a);
        }
        return makePure(res);
      }});
}

template <typename A>
Parser<Many<A>> many1(const Parser<A>& p)
{
    Parser<Many<A>> manyP = many<A>(p);

    return bind<A, Many<A>>(p, [=](const A& a) {
        return fmap<Many<A>, Many<A>>([=](Many<A> rest) {
            rest.push_front(a);
            return rest;
        }, manyP);
    });
}

template <typename A>
Parser<ParserResult<A>> tryOrError(const Parser<A> &p)
{
    Parser<A> itemCopy = p;

    auto pCopy = std::make_shared<Parser<Any>>(
        fmap<A, Any>([](const A &a)
                     { return a; }, itemCopy));

    return makeFree(TryOrErrorParser<Parser<ParserResult<A>>>{
        pCopy,
        [](const ParserResult<Any> &res)
        {
            if (isRight(res))
            {
                A a = std::any_cast<A>(getParseSucceeded(res).parsed);
                return makePure(ParserResult<A>{ParserSucceeded<A>{a, getParseSucceeded(res).from, getParseSucceeded(res).to}});
            }
            else
            {
                ParserFailed failed = std::get<ParserFailed>(res);
                return makePure(ParserResult<A>{ParserFailed{failed.message, failed.at}});
            }
        }
    });
}

template <typename A>
Parser<A> tryOrThrow(const Parser<A> &p)
{
    return bind<ParserResult<A>, A>(tryOrError(p), [](const ParserResult<A>& res) {
        if (isRight(res))
        {
            return makePure(std::any_cast<A>(getParseSucceeded(res).parsed));
        }
        else
        {
            ParserFailed failed = std::get<ParserFailed>(res);
            throw std::runtime_error("Parsing failed at position " + std::to_string(failed.at) + ": " + failed.message);
        }
    });
}

template <typename A>
Parser<A> try_(const Parser<A> &p)
{
    return tryOrThrow(p);
}

template <typename A>
Parser<A> alt(const Parser<A> &p, const Parser<A> &q)
{
  auto pCopy = std::make_shared<Parser<Any>>(
      fmap<A, Any>([](const A &a)
                   { return a; }, p));

  auto qCopy = std::make_shared<Parser<Any>>(
      fmap<A, Any>([](const A &a)
                   { return a; }, q));

  return makeFree(AltParser<Parser<A>>{
      pCopy,
      qCopy,
      [](const Any &res)
      {
        A a = std::any_cast<A>(res);
        return makePure(a);
      }});
}


template <typename A>
Parser<A> lazy(const std::function<Parser<A>()>& parserFactory)
{
    std::function<Parser<Any>()> parserFactoryAny = [=]() {
        Parser<A> actualParser = parserFactory();
        return fmap<A, Any>([](const A &a)
                            { return a; }, actualParser);
    };
    return makeFree(LazyParser<Parser<A>>{
        parserFactoryAny,
        [](const Any& res)
        {
            A a = std::any_cast<A>(res);
            return makePure(a);
        }
    });
}

// seq: run two parsers in sequence and return the result of the second one
template <typename A, typename B>
Parser<B> seq(const Parser<A>& p, const Parser<B>& q)
{
    return bind<A, B>(p, [=](const A&) { return q; });
}

// TODO: test it
template <typename A>
Parser<Many<A>> bothSequence(const Parser<A>& fst, const Parser<A>& snd)
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
Parser<std::pair<A, B>> both(const Parser<A>& fst, const Parser<B>& snd)
{
    return bind<A, std::pair<A, B>>(fst, [=](const A& a) {
        return fmap<B, std::pair<A, B>>([=](const B& b) {
            return std::make_pair(a, b);
        }, snd);
    });
}

template <typename A, typename B>
Parser<A> left(const Parser<A>& p, const Parser<B>& q)
{
    return bind<A, A>(p, [=](const A& a) {
        return fmap<B, A>([=](const B&) { return a; }, q);
    });
}

template <typename A, typename B>
Parser<B> right(const Parser<A>& p, const Parser<B>& q)
{
    return bind<A, B>(p, [=](const A&) { return q; });
}


template <typename A, typename S>
Parser<Many<A>> sepBy1(const Parser<A>& item, const Parser<S>& sep)
{
    // sepThenItem = sep *> item
    Parser<A> sepThenItem = bind<S, A>(sep, [=](const S&) { return item; });

    Parser<A> itemCopy = item;
    Parser<Many<A>> restParser = many<A>(sepThenItem);

    return bind<A, Many<A>>(item, [=](const A& first) {
        return fmap<Many<A>, Many<A>>([=](Many<A> rest) {
            rest.push_front(first);
            return rest;
        }, restParser);
    });
}

template <typename O, typename A, typename C>
Parser<A> between(const Parser<O>& open, const Parser<A>& content, const Parser<C>& close)
{
    return bind<O, A>(open, [=](const O&) {
        return bind<A, A>(content, [=](const A& a) {
            return bind<C, A>(close, [=](const C&) {
                return makePure(a);
            });
        });
    });
}

template <typename A>
Parser<Many<A>> count(size_t n, const Parser<A>& p)
{
    if (n == 0)
    {
        return makePure(Many<A>{});
    }

    // create a copy for passing to many/count recursively
    Parser<A> pCopy = p;

    return bind<A, Many<A>>(p, [=](const A& a) {
        return fmap<Many<A>, Many<A>>([=](Many<A> rest) {
            rest.push_front(a);
            return rest;
        }, count(n - 1, pCopy));
    });
}

template <typename A, typename B, typename R>
Parser<R> liftA2(const std::function<R(A, B)>& f, const Parser<A>& pa, const Parser<B>& pb)
{
    return bind<A, R>(pa, [=](const A& a) {
        return fmap<B, R>([=](const B& b) {
            return f(a, b);
        }, pb);
    });
}

// This combinator needs some investigation.
// template <typename A, typename R>
// Parser<R> ap(const Parser<std::function<R(A)>>& pf, const Parser<A>& pa)
// {
//     return bind<std::function<R(A)>, R>(pf, [=](const std::function<R(A)>& f) {
//         return fmap<A, R>(f, pa);
//     });
// }

template <typename A>
Parser<Unit> discard(const Parser<A>& p)
{
    return fmap<A, Unit>([=](const A&) { return unit; }, p);
}


// Some specific combinators

template <typename Dummy = int>
Parser<std::string> manyCharsToString(
  const Parser<Many<Char>> &manyCharsParser)
{
  return fmap<Many<Char>, std::string>(
      [](const Many<Char>& chars) {
          return std::string(chars.begin(), chars.end());
      },
      manyCharsParser);
}

// Combinators for building sequences of parsers and mapping them into structs using aggregate initialization.

template <typename A>
Parser<Unit> skip(const Parser<A> &p)
{
  return fmap<A, Unit>([](const A &)
                       { return unit; }, p);
}

// Variadic sequence combinator and helpers

// Helper to detect Unit
template <typename T>
struct is_unit : std::is_same<T, Unit> {};

// filter_units<Ts...>::type yields std::tuple of Ts with Unit removed
template <typename... Ts>
struct filter_units;

template <>
struct filter_units<> {
    using type = std::tuple<>;
};

template <typename T, typename... Ts>
struct filter_units<T, Ts...>
{
    using tail_type = typename filter_units<Ts...>::type;
    using type = std::conditional_t<is_unit<T>::value,
                                   tail_type,
                                   decltype(std::tuple_cat(std::declval<std::tuple<T>>(), std::declval<tail_type>()))>;
};

// sequence: run parsers in order and return tuple of results, omitting Unit values

// Base: zero parsers -> pure empty tuple
inline Parser<std::tuple<>> sequence()
{
    return makePure(std::tuple<>{});
}

// Single parser
/**
 * @brief Transforms a parser to return a tuple with filtered unit types.
 *
 * This function takes a parser `p` that produces a value of type `A` and returns a new parser
 * that produces a tuple of the filtered type (with unit types removed). If the parser returns
 * a unit type, the result is mapped to an empty tuple. Otherwise, the result is mapped to a
 * single-element tuple containing the parsed value.
 *
 * @tparam A The type produced by the input parser.
 * @param p The input parser producing a value of type `A`.
 * @return Parser<typename filter_units<A>::type> A parser producing a tuple with unit types filtered out.
 */
template <typename A>
Parser<typename filter_units<A>::type> sequence(const Parser<A>& p)
{
    using OutTuple = typename filter_units<A>::type;

    if constexpr (is_unit<A>::value)
    {
        // parser returns Unit -> map to empty tuple
        return fmap<A, OutTuple>([](const A&) { return OutTuple{}; }, p);
    }
    else
    {
        return fmap<A, OutTuple>([](const A& a) { return std::tuple<A>(a); }, p);
    }
}

// Multiple parsers

/**
 * @brief Sequence combinator for parsers, filtering out unit types.
 *
 * This function composes multiple parsers (`p`, `rest...`) into a single parser that applies them in sequence.
 * The results of each parser are combined into a tuple, but any result of type `unit` (as determined by `is_unit`)
 * is omitted from the output tuple. This is useful for ignoring intermediate results that are not meaningful,
 * such as those produced by parsers for whitespace or punctuation.
 *
 * The combinator works recursively: it binds the first parser, then sequences the rest, and finally combines
 * their results. If the first parser's result is a unit, it is omitted; otherwise, it is prepended to the tuple
 * of results from the rest.
 *
 * @tparam A The type of the first parser's result.
 * @tparam Rest The types of the remaining parsers' results.
 * @param p The first parser in the sequence.
 * @param rest The remaining parsers to be sequenced.
 * @return Parser<typename filter_units<A, Rest...>::type> A parser that returns a tuple of non-unit results.
 *
 * @note This combinator is useful for building complex parsers where only certain results are significant,
 *       and others (like structural tokens) can be ignored.
 */
template <typename A, typename... Rest>
Parser<typename filter_units<A, Rest...>::type> sequence(const Parser<A>& p, const Parser<Rest>&... rest)
{
    using OutTuple = typename filter_units<A, Rest...>::type;
    using RestTuple = typename filter_units<Rest...>::type;

    return bind<A, OutTuple>(p, [=](const A& a) {
        Parser<RestTuple> restP = sequence(rest...);
        return fmap<RestTuple, OutTuple>([=](const RestTuple& rt) -> OutTuple {
            if constexpr (is_unit<A>::value)
            {
                return rt;
            }
            else
            {
                return std::tuple_cat(std::tuple<A>(a), rt);
            }
        }, restP);
    });
}

// as<Target>: map a Parser<std::tuple<...>> into Parser<Target> using aggregate-initialization
template <typename Target, typename Tuple>
Parser<Target> as(const Parser<Tuple>& p)
{
    return fmap<Tuple, Target>([](const Tuple& t) {
        return std::apply([](auto&&... args) -> Target { return Target{std::forward<decltype(args)>(args)...}; }, t);
    }, p);
}


} // namespace core
} // namespace ps

#endif // PS_CORE_PARSER_H
