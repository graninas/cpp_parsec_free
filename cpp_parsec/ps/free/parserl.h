#ifndef PS_FREE_PSL_H
#define PS_FREE_PSL_H

#include "../types.h"
#include "../psf/parser_adt.h"

namespace ps
{
namespace free
{
// PS Free

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
  psf::ParserADT<ParserL<Ret>> psf;
};

// Recursive Free ParserL algebraic data type

template <typename A>
struct ParserL
{
    std::variant<PureF<A>, FreeF<A>> psl;
};


// Previously: purePL
template <typename A>
ParserL<A> make_pure(const A& a, Pos from, Pos to)
{
    return { PureF<A>{ a, from, to } };
}

// Previously: wrap
template <typename A,
          template <typename> class Method>
ParserL<A> make_free(const Method<ParserL<A>> &method)
{
  return { FreeF<A>{psf::ParserADT<ParserL<A>>{method}} };
}

} // namespace free
} // namespace ps

#endif // PS_FREE_PSL_H

