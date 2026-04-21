#ifndef PS_CORE_PARSER_H
#define PS_CORE_PARSER_H

#include "engine.h"
#include "conds.h"

#include <tuple>
#include <type_traits>
#include <utility>

namespace ps
{
namespace core
{


// Generic

template <typename A, typename B>
Parser<B> applicative(const Parser<std::function<B(A)>> &pf,
                      const Parser<A> &pa)
{
  return bind<std::function<B(A)>, B>(
      pf, [=](const std::function<B(A)> &f)
      { return fmap<A, B>(f, pa); });
}

// Parsers

template <typename Dummy = int>
Parser<Char> parseSymbolCond(
        const std::function<bool(char)>& cond)
{
    std::function<bool(char)> cond_copy = cond;

    std::function<bool(Any)> condAny = [=](const Any& any)  {
        char ch = std::any_cast<char>(any);
        return cond_copy(ch);
    };

    return wrap<Char, ParseSymbolCond<Parser<Char>, Parser>>
      (ParseSymbolCond<Parser<Char>, Parser>{
                      condAny,
                      [](const Any& any)  {
                          char ch = std::any_cast<char>(any);
                          {
                              // return pure(ch, "Pure of parseSymbolCond");
                              return pure(ch, "");
                          }
                      }
                }, "parseSymbolCond");
}

template <typename Dummy = int>
Parser<Char> parseChar(char expected)
{
    return parseSymbolCond([=](char ch) { return ch == expected; })
      + (std::string("parseChar '") + expected + "'");
}

template <typename Dummy = int>
Parser<std::string> parseLit(const std::string& s)
{
    return wrap<std::string, ParseLit<Parser<std::string>, Parser>>
      (ParseLit<Parser<std::string>, Parser>{
                      s,
                      [](const std::string& resS) {
                          return pure(resS, "");
                          // return pure(resS, "Pure of parseLit");
                      }
                }, "parseLit \"" + s + "\"");
}


const Parser<Char> digit = parseSymbolCond(isDigit) + "digit";
const Parser<Char> upper = parseSymbolCond(isUpper) + "upper";
const Parser<Char> lower = parseSymbolCond(isLower) + "lower";
const Parser<Char> alpha = parseSymbolCond(isAlpha) + "alpha";
const Parser<Char> alphanum = parseSymbolCond(isAlphanum) + "alphanum";
const Parser<Char> space = parseSymbolCond(isSpace) + "space";
const Parser<Char> anyChar = parseSymbolCond([](char) { return true; }) + "any char";
const Parser<Char> eol = parseSymbolCond(isEol) + "eol";
const Parser<Char> cr = parseSymbolCond(isCr) + "cr";
const Parser<Char> comma = parseSymbolCond([](char ch)
                                             { return ch == ','; }) + "comma";

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
        // return pure(res, "Pure of many of " + itemCopy.debugInfo);
        return pure(res, "");
      }}, "many of " + itemCopy.debugInfo);
}

template <typename A>
Parser<Many<A>> many1(const Parser<A>& p)
{
    Parser<Many<A>> manyP = many<A>(p) + "many in many1 of " + p.debugInfo;

    return bind<A, Many<A>>(p, [=](const A& a) {
        return fmap<Many<A>, Many<A>>([=](Many<A> rest) {
            rest.push_front(a);
            return rest;
        }, manyP);
    }) + ("many1 of " + p.debugInfo);
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
                  ParserResult<A>{ParserSucceeded<A>{a, getParseSucceeded(res).from, getParseSucceeded(res).to}},
                  "");
                  // "Pure of tryOrError of " + itemCopy.debugInfo);
            }
            else
            {
                ParserFailed failed = std::get<ParserFailed>(res);
                return pure(ParserResult<A>{ParserFailed{failed.message, failed.at}},
                  // "Pure of tryOrError of " + itemCopy.debugInfo);
                  "");
            }
        }
      }, "tryOrError of " + itemCopy.debugInfo);
}

template <typename A>
Parser<A> tryOrThrow(const Parser<A> &p)
{
    return bind<ParserResult<A>, A>(tryOrError(p), [=](const ParserResult<A>& res) {
        if (isRight(res))
        {
            return pure(std::any_cast<A>(getParseSucceeded(res).parsed),
              "");
              // "Pure of tryOrThrow of " + p.debugInfo);
        }
        else
        {
            ParserFailed failed = std::get<ParserFailed>(res);
            throw std::runtime_error("Parsing failed at position " + std::to_string(failed.at) + ": " + failed.message);
        }
    }) + ("tryOrThrow of " + p.debugInfo);
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

  return wrap<A, AltParser<Parser<A>, Parser>>
    (AltParser<Parser<A>, Parser>{
      pCopy,
      qCopy,
      [=](const Any &res)
      {
        A a = std::any_cast<A>(res);
        return pure(a, "");
        // return pure(a, "Pure of alt of " + p.debugInfo + " and " + q.debugInfo);
      }}, "alt of " + p.debugInfo + " and " + q.debugInfo);
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
            // return pure(a, "Pure of lazy parser");
        }
    }, "lazy");
}

// seq: run two parsers in sequence and return the result of the second one
template <typename A, typename B>
Parser<B> seq(const Parser<A>& p, const Parser<B>& q)
{
    return bind<A, B>(p, [=](const A&) { return q; }) +
      ("seq of " + p.debugInfo + " and " + q.debugInfo);
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
    }) + ("bothSequence of " + fst.debugInfo + " and " + snd.debugInfo);
}

// TODO: test it
template <typename A, typename B>
Parser<std::pair<A, B>> both(const Parser<A>& fst, const Parser<B>& snd)
{
    return bind<A, std::pair<A, B>>(fst, [=](const A& a) {
        return fmap<B, std::pair<A, B>>([=](const B& b) {
            return std::make_pair(a, b);
        }, snd);
    }) + ("both of " + fst.debugInfo + " and " + snd.debugInfo);
}

template <typename A, typename B>
Parser<A> left(const Parser<A>& p, const Parser<B>& q)
{
    return bind<A, A>(p, [=](const A& a) {
        return fmap<B, A>([=](const B&) { return a; }, q);
    }) + ("left of " + p.debugInfo + " and " + q.debugInfo);
}

template <typename A, typename B>
Parser<B> right(const Parser<A>& p, const Parser<B>& q)
{
    return bind<A, B>(p, [=](const A&) { return q; }) +
      ("right of " + p.debugInfo + " and " + q.debugInfo);
}


template <typename A, typename S>
Parser<Many<A>> sepBy1(const Parser<A>& item, const Parser<S>& sep)
{
    // sepThenItem = sep *> item
    Parser<A> sepThenItem = bind<S, A>(sep, [=](const S&) { return item; })
      + ("sepThenItem of " + sep.debugInfo + " and " + item.debugInfo);

    Parser<Many<A>> restParser = many<A>(sepThenItem)
      + ("restParser (many) of sepBy1 of " + item.debugInfo + " and " + sep.debugInfo);

    return bind<A, Many<A>>(item, [=](const A& first) {
        return fmap<Many<A>, Many<A>>([=](Many<A> rest) {
            rest.push_front(first);
            return rest;
        }, restParser);
    }) + ("sepBy1 of " + item.debugInfo + " and " + sep.debugInfo);
}

template <typename O, typename A, typename C>
Parser<A> between(const Parser<O>& open, const Parser<A>& content, const Parser<C>& close)
{
    return bind<O, A>(open, [=](const O&) {
        return bind<A, A>(content, [=](const A& a) {
            return bind<C, A>(close, [=](const C&) {
                // return pure(a, "Pure of between of " + open.debugInfo + " and " + close.debugInfo + " with content " + content.debugInfo);
                return pure(a, "");
            });
        });
    }) + ("between of " + open.debugInfo + " and " + close.debugInfo + " with content " + content.debugInfo);
}

template <typename A>
Parser<Many<A>> count(size_t n, const Parser<A>& p)
{
    if (n == 0)
    {
        // return pure(Many<A>{}, "Pure of count 0 of " + p.debugInfo);
        return pure(Many<A>{}, "");
    }

    // create a copy for passing to many/count recursively
    Parser<A> pCopy = p;

    return bind<A, Many<A>>(p, [=](const A& a) {
        return fmap<Many<A>, Many<A>>([=](Many<A> rest) {
            rest.push_front(a);
            return rest;
        }, count(n - 1, pCopy));
    }) + ("count of " + p.debugInfo);
}

template <typename A, typename B, typename R>
Parser<R> liftA2(const std::function<R(A, B)>& f, const Parser<A>& pa, const Parser<B>& pb)
{
    return bind<A, R>(pa, [=](const A& a) {
        return fmap<B, R>([=](const B& b) {
            return f(a, b);
        }, pb);
    }) + ("liftA2 of " + pa.debugInfo + " and " + pb.debugInfo);
}

template <typename A>
Parser<Unit> discard(const Parser<A>& p)
{
    return fmap<A, Unit>([=](const A&) { return unit; }, p) +
      ("discard of " + p.debugInfo);
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
                       { return unit; }, p) +
                       ("skip of " + p.debugInfo);
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
    // return pure(std::tuple<>{}, "Pure of empty sequence");
    return pure(std::tuple<>{}, "");
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

// Variadic template implementation of choice
// Tries each parser in sequence and returns the result of the first one that succeeds.
template <typename Parser1, typename... Parsers>
Parser<typename Parser1::ResultType> choice(const Parser1 &p1, const Parsers &...parsers)
{
  if constexpr (sizeof...(parsers) == 0)
  {
    return p1;
  }
  else
  {
    return alt(try_(p1), choice(parsers...));
  }
}

} // namespace core
} // namespace ps

#endif // PS_CORE_PARSER_H
