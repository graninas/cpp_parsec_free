#ifndef PS_CORE_PARSER_ADT_H
#define PS_CORE_PARSER_ADT_H

#include "../types.h"

namespace ps
{
namespace core
{

  // Forward declaration
  template <typename A>
  struct ParserL;

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
    std::shared_ptr<ParserL<Any>> raw_parser;   // Parser is always the same
    std::function<Next(std::list<Any>)> next;
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
    std::variant<
        ParseSymbolCond<Ret>,
        ParseMany<Ret>,
        ParseLit<Ret>,
        GetSt<Ret>,
        PutSt<Ret>
    > psf;
};

// Free language


// Free methods

template <typename Ret>
struct PureF
{
  Ret ret;
};

template <typename Ret>
struct FreeF
{
  ParserADT<ParserL<Ret>> psf;
};

template <typename A>
struct ParserL
{
  std::variant<PureF<A>, FreeF<A>> psl;
};


template <typename A>
ParserL<A> make_pure(const A &a)
{
  return {PureF<A>{a}};
}

template <typename A,
          template <typename> class Method>
ParserL<A> make_free(const Method<ParserL<A>> &method)
{
  return {FreeF<A>{ParserADT<ParserL<A>>{method}}};
}

template <typename A>
ParserL<A> pure(const A &a)
{
  return make_pure(a);
}

} // namespace core
} // namespace ps

#endif // PS_CORE_PARSER_ADT_H
