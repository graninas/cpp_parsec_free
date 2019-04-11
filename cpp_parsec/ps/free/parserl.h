#ifndef PS_FREE_PSL_H
#define PS_FREE_PSL_H

#include "../types.h"
#include "../psf/parserf.h"

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
};

template <typename Ret>
struct FreeF
{
    psf::ParserF<ParserL<Ret>> psf;
};

// Recursive Free ParserL algebraic data type

template <typename A>
struct ParserL
{
    std::variant<PureF<A>, FreeF<A>> psl;
};


template <typename A>
ParserL<A> runPurePL(const A& a)
{
    return { PureF<A>{ a } };
}

} // namespace free
} // namespace ps

#endif // PS_FREE_PSL_H

