#ifndef PS_CHURCH_PSL_H
#define PS_CHURCH_PSL_H

#include "../types.h"
#include "../psf/parserf.h"
#include "../psf/functor.h"

namespace ps
{
namespace church
{

// PS Free Church-encoded

template <typename A>
using ParserFunc = std::function<
        S(
            std::function<S(A)>,
            std::function<S(psf::ParserF<S>)>
        )>;

template <typename A>
struct ParserL
{
    ParserFunc<A> runF;
};

} // namespace church
} // namespace ps

#endif // PS_CHURCH_PSL_H

