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
struct Parser;

// Free methods

template <typename Ret>
struct PureF
{
    Ret ret;
};

template <typename Ret>
struct FreeF
{
    psf::ParserF<Parser<Ret>> psf;
};

// Recursive Free Parser algebraic data type

template <typename A>
struct Parser
{
    std::variant<PureF<A>, FreeF<A>> psl;
};


template <typename A>
Parser<A> runPurePL(const A& a)
{
    return { PureF<A>{ a } };
}

} // namespace free
} // namespace ps

#endif // PS_FREE_PSL_H

