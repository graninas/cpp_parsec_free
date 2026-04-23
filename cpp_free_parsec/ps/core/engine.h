#ifndef PS_CORE_ENGINE_H
#define PS_CORE_ENGINE_H

// #define FREE
#define CHURCH

#include "types.h"
#include "methods/adt.h"
#include "methods/functor.h"

#ifdef FREE

#include "free/adt.h"
#include "free/functor.h"
#include "free/bind.h"

#endif // FREE

#ifdef CHURCH

#include "church/adt.h"
#include "church/functor.h"
#include "church/bind.h"

#endif // CHURCH

namespace ps
{
namespace core
{


#ifdef FREE

template <typename A>
using Parser = ps::core::free::Parser<A>;


// TODO: debugInfo is blank by default
// template <typename A,
//           template <typename, template <typename> class> class Method>
template <typename A,
          typename Method>
Parser<A> wrap(const Method &method, const std::string &debugInfo)
{
  return ps::core::free::makeFree<A, Method>
    (method, debugInfo);
}

// TODO: debugInfo is blank by default
template <typename A>
Parser<A> pure(const A& a, const std::string& debugInfo)
{
  return ps::core::free::makePure<A>(a, debugInfo);
}

template <typename A, typename B>
Parser<B> fmap(
    const std::function<B(A)> &f,
    const Parser<A> &psl)
{
  return ps::core::free::runFMap<A,B>(f, psl);
}


template <typename A, typename B>
Parser<B> bind(const Parser<A> &ma,
               const std::function<Parser<B>(A)> &f)
{
  return ps::core::free::runBind<A, B>(ma, f);
}

// Methods

template <typename Dummy = int>
Parser<Char> parseSymbolCond(
    const std::function<bool(char)> &cond)
{
  std::function<bool(char)> cond_copy = cond;

  std::function<bool(Any)> condAny = [=](const Any& any)  {
      char ch = std::any_cast<char>(any);
      return cond_copy(ch);
  };

  return wrap<Char, ParseSymbolCond<Parser<Char>, Parser>>
    (ParseSymbolCond<Parser<Char>, Parser>{
                    condAny,
                    [](const Any& any)
                    {
                        char ch = std::any_cast<char>(any);
                        return pure(ch, "");
                    }
              }, "parseSymbolCond");
}

template <typename Dummy = int>
Parser<std::string> parseLit(const std::string &s)
{
  return wrap<std::string, ParseLit<Parser<std::string>, Parser>>
  (ParseLit<Parser<std::string>, Parser>{
                  s,
                  [](const std::string& resS) {
                      return pure(resS, "");
                  }
            }, "parseLit \"" + s + "\"");
}

template <typename A>
Parser<Many<A>> many(const Parser<A>& item)
{
  Parser<A> itemCopy = item;

  auto pCopy = std::make_shared<Parser<Any>>(
      fmap<A, Any>([](const A &a)
                   { return a; }, itemCopy));

  return wrap<Many<A>, ParseMany<Parser<Many<A>>, Parser>>
    (ParseMany<Parser<Many<A>>, Parser>{
      pCopy,
      [=](const std::list<Any> &resList)
      {
        Many<A> res;
        for (const Any &any : resList)
        {
          A a = std::any_cast<A>(any);
          res.push_back(a);
        }
        return pure(res, "");
      }}, "many[" + itemCopy.debugInfo + "]");
}

template <typename A>
Parser<ParserResult<A>> tryOrError(const Parser<A> &p)
{
    Parser<A> itemCopy = p;

    auto pCopy = std::make_shared<Parser<Any>>(
        fmap<A, Any>([](const A &a)
                     { return a; }, itemCopy));

    return wrap<ParserResult<A>, TryOrErrorParser<Parser<ParserResult<A>>, Parser>>
      (TryOrErrorParser<Parser<ParserResult<A>>, Parser>{
        pCopy,
        [=](const ParserResult<Any> &res)
        {
            if (isRight(res))
            {
                A a = std::any_cast<A>(getParseSucceeded(res).parsed);
                return pure(
                  ParserResult<A>{ParserSucceeded<A>{a, getParseSucceeded(res).from, getParseSucceeded(res).to}}, "");
            }
            else
            {
                ParserFailed failed = std::get<ParserFailed>(res);
                return pure(ParserResult<A>{ParserFailed{failed.message, failed.at}}, "");
            }
        }
      }, "try[" + itemCopy.debugInfo + "]");
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

  return wrap<A, AltParser<Parser<A>, Parser>>
    (AltParser<Parser<A>, Parser>{
      pCopy,
      qCopy,
      [=](const Any &res)
      {
        A a = std::any_cast<A>(res);
        return pure(a, "");
      }}, "alt[" + p.debugInfo + ";" + q.debugInfo + "]");
}

template <typename A>
Parser<A> lazy(const std::function<Parser<A>()>& parserFactory)
{
    std::function<Parser<Any>()> parserFactoryAny = [=]() {
        Parser<A> actualParser = parserFactory();
        return fmap<A, Any>([](const A &a)
                            { return a; }, actualParser);
    };

    return wrap<A, LazyParser<Parser<A>, Parser>>
      (LazyParser<Parser<A>, Parser>{
        parserFactoryAny,
        [](const Any& res)
        {
            A a = std::any_cast<A>(res);
            return pure(a, "");
        }
    }, "lazy");
}

#endif // FREE



#ifdef CHURCH

template <typename A>
using Parser = ps::core::church::Parser<A>;


// TODO: debugInfo is blank by default
template <typename A,
          typename Method>
Parser<A> wrap(const Method& method, const std::string& debugInfo)
{
  return ps::core::church::makeChurch<A, Method>
    (method, debugInfo);
}

// TODO: debugInfo is blank by default
template <typename A>
Parser<A> pure(const A& a, const std::string& debugInfo)
{
  return ps::core::church::makePure<A>(a, debugInfo);
}


template <typename A, typename B>
Parser<B> fmap(
    const std::function<B(A)> &f,
    const Parser<A> &psl)
{
  return ps::core::church::runFMap<A,B>(f, psl);
}


template <typename A, typename B>
Parser<B> bind(const Parser<A> &ma,
               const std::function<Parser<B>(A)> &f)
{
  return ps::core::church::runBind<A, B>(ma, f);
}


// Methods

template <typename Dummy = int>
Parser<Char> parseSymbolCond(
    const std::function<bool(char)> &cond)
{
  std::function<bool(char)> cond_copy = cond;

  std::function<bool(Any)> condAny = [=](const Any& any)  {
      char ch = std::any_cast<char>(any);
      return cond_copy(ch);
  };

  return wrap<Char, ParseSymbolCond<Char, Parser>>
    (ParseSymbolCond<Char, Parser>{
                    condAny,
                    [](const Any& any) {
                        return std::any_cast<char>(any);
                    }
              }, "parseSymbolCond");
}


template <typename Dummy = int>
Parser<std::string> parseLit(const std::string &s)
{
  return wrap<std::string, ParseLit<std::string, Parser>>
  (ParseLit<std::string, Parser>{
                  s,
                  [](const std::string& resS) {
                      return resS;
                  }
            }, "parseLit \"" + s + "\"");
}

template <typename A>
Parser<Many<A>> many(const Parser<A>& item)
{
  Parser<A> itemCopy = item;

  auto pCopy = std::make_shared<Parser<Any>>(
      fmap<A, Any>([](const A &a)
                   { return a; }, itemCopy));

  return wrap<Many<A>, ParseMany<Many<A>, Parser>>
    (ParseMany<Many<A>, Parser>{
      pCopy,
      [=](const std::list<Any> &resList)
      {
        Many<A> res;
        for (const Any &any : resList)
        {
          A a = std::any_cast<A>(any);
          res.push_back(a);
        }
        return res;
      }}, "many[" + itemCopy.debugInfo + "]");
}

template <typename A>
Parser<ParserResult<A>> tryOrError(const Parser<A> &p)
{
    Parser<A> itemCopy = p;

    auto pCopy = std::make_shared<Parser<Any>>(
        fmap<A, Any>([](const A &a)
                     { return a; }, itemCopy));

    return wrap<ParserResult<A>, TryOrErrorParser<ParserResult<A>, Parser>>
      (TryOrErrorParser<ParserResult<A>, Parser>{
        pCopy,
        [=](const ParserResult<Any> &res)
        {
            if (isRight(res))
            {
                A a = std::any_cast<A>(getParseSucceeded(res).parsed);
                return
                  ParserResult<A>{ParserSucceeded<A>{a, getParseSucceeded(res).from, getParseSucceeded(res).to}};
            }
            else
            {
                ParserFailed failed = std::get<ParserFailed>(res);
                return ParserResult<A>{ParserFailed{failed.message, failed.at}};
            }
        }
      }, "tryOrErr[" + itemCopy.debugInfo + "]");
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

  return wrap<A, AltParser<A, Parser>>
    (AltParser<A, Parser>{
      pCopy,
      qCopy,
      [=](const Any &res) {
        A a = std::any_cast<A>(res);
        return a;
      }}, "alt[" + p.debugInfo + ";" + q.debugInfo + "]");
}

template <typename A>
Parser<A> lazy(const std::function<Parser<A>()>& parserFactory)
{
    std::function<Parser<Any>()> parserFactoryAny = [=]() {
        Parser<A> actualParser = parserFactory();
        return fmap<A, Any>([](const A &a)
                            { return a; }, actualParser);
    };

    return wrap<A, LazyParser<A, Parser>>
      (LazyParser<A, Parser>{
        parserFactoryAny,
        [](const Any& res) {
            A a = std::any_cast<A>(res);
            return a;
        }
    }, "lazy");
}


#endif // CHURCH


} // namespace core
} // namespace ps

#endif // PS_CORE_ENGINE_H
