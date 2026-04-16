#ifndef PS_CORE_ADT_PARSER_H
#define PS_CORE_ADT_PARSER_H

#include "../types.h"
#include "methods.h"

namespace ps
{
namespace core
{

// Forward declaration
template <typename A>
struct ParserL;

// Free methods

template <typename Ret>
struct PureF
{
    Ret ret;
    Pos from;
    Pos to;
};

template <typename Ret>
struct FreeF
{
  ParserADT<ParserL<Ret>> psf;
};

// Recursive Free ParserL algebraic data type

template <typename A>
struct ParserL
{
    std::variant<PureF<A>, FreeF<A>> psl;
};


template <typename A>
ParserL<A> make_pure(const A& a, Pos from, Pos to)
{
    return { PureF<A>{ a, from, to } };
}

template <typename A,
          template <typename> class Method>
ParserL<A> make_free(const Method<ParserL<A>> &method)
{
  return { FreeF<A>{ParserADT<ParserL<A>>{method}} };
}

} // namespace core
} // namespace ps

#endif // PS_CORE_ADT_PARSER_H

