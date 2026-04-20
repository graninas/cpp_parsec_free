#ifndef PS_CORE_PARSER_ADT_H
#define PS_CORE_PARSER_ADT_H

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
    std::string name;
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

// Free language


// Free methods

template <typename Ret>
struct PureF
{
  using ResultType = Ret;
  Ret ret;
};

template <typename Ret>
struct FreeF
{
  using ResultType = Ret;
  ParserADT<Parser<Ret>> psf;
};

template <typename A>
struct Parser
{
  using ResultType = A;
  std::variant<PureF<A>, FreeF<A>> psl;
};


template <typename A>
Parser<A> makePure(const A &a)
{
  return {PureF<A>{a}};
}

template <typename A,
          template <typename> class Method>
Parser<A> makeFree(const Method<Parser<A>> &method)
{
  return {FreeF<A>{ParserADT<Parser<A>>{method}}};
}

template <typename A>
Parser<A> pure(const A &a)
{
  return makePure(a);
}

} // namespace core
} // namespace ps

#endif // PS_CORE_PARSER_ADT_H
