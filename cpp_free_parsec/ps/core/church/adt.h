#ifndef PS_CORE_CHURCH_ADT_H
#define PS_CORE_CHURCH_ADT_H

#include "../types.h"

namespace ps
{
namespace core
{

  // Forward declaration
  template <typename A>
  struct Parser;

  // PS methods

  template <typename Next>
  struct ParseSymbolCond
  {
    std::function<bool(Any)> validator;
    std::function<Next(Any)> next;
  };

  template <typename Next>
  struct ParseMany
  {
    std::shared_ptr<Parser<Any>> rawParser;   // Parser is always the same
    std::function<Next(std::list<Any>)> next;
  };

  template <typename Next>
  struct TryOrErrorParser
  {
    std::shared_ptr<Parser<Any>> rawParser;   // Parser is always the same
    std::function<Next(ParserResult<Any>)> next;
  };

  template <typename Next>
  struct AltParser
  {
    std::shared_ptr<Parser<Any>> p;   // Parser is always the same
    std::shared_ptr<Parser<Any>> q;   // Parser is always the same
    std::function<Next(Any)> next;
  };

  template <typename Next>
  struct LazyParser
  {
    std::function<Parser<Any>()> parserFactory;   // Factory is always the same
    std::function<Next(Any)> next;
  };


  template <typename Next>
  struct ParseLit
  {
    std::string s;
    std::function<Next(std::string)> next;
  };

  template <typename Next>
  struct GetSt
  {
    std::function<Next(State)> next;
  };

  template <typename Next>
  struct PutSt
  {
      State st;
      std::function<Next(Unit)> next;
  };


  template <class Ret>
  struct ParserADT        // TODO: rename to Methods
  {
    using ResultType = Ret;
    std::variant<
        ParseSymbolCond<Ret>,
        ParseMany<Ret>,
        ParseLit<Ret>,
        GetSt<Ret>,
        PutSt<Ret>,
        TryOrErrorParser<Ret>,
        AltParser<Ret>,
        LazyParser<Ret>>
        psf;
  };

// Church Free language


// Church Free methods

template <typename Ret>
using Continuation = std::function<
    Any(
        std::function<Any(Ret)>,
        std::function<Any(ParserADT<Any>)>)>;

template <typename A>
struct Parser
{
  using ResultType = A;
  Continuation<A> runF;
  std::string debugInfo;

  Parser<A> operator+(const std::string &info) const
  {
    Parser<A> newParser = *this;
    newParser.debugInfo = info;
    return newParser;
  }
};

template <typename A>
Parser<A> pure(const A &a, const std::string &debugInfo)
{
  Parser<A> n;
  n.debugInfo = debugInfo;
  n.runF = [=](const std::function<Any(A)> &p,
               const std::function<Any(Parser<Any>)> &)
  {
    return p(a);
  };
  return n;
}

template <typename A, template <typename, typename> class Method>
Parser<A> wrap(const Method<Any, A> &method)
{
  Parser<A> n;

  n.runF = [=](const std::function<Any(A)> &p,
               const std::function<Any(Parser<Any>)> &r)
  {
    Parser<A> f{method};
    Parser<Any> mapped = fmap<A, Any>(p, f);
    return r(mapped);
  };

  return n;
}

// template <typename A>
// Parser<A> makePure(const A &a, const std::string& debugInfo)
// {
//   return {PureF<A>{a}, debugInfo};
// }

// template <typename A,
//           template <typename> class Method>
// Parser<A> makeFree(const Method<Parser<A>> &method, const std::string& debugInfo)
// {
//   return {FreeF<A>{ParserADT<Parser<A>>{method}}, debugInfo};
// }

// template <typename A>
// Parser<A> pure(const A &a, const std::string& debugInfo)
// {
//   return makePure(a, debugInfo);
// }

} // namespace core
} // namespace ps

#endif // PS_CORE_CHURCH_ADT_H
