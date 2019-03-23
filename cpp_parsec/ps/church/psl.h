#ifndef PS_CHURCH_PSL_H
#define PS_CHURCH_PSL_H

#include "../tvar.h"
#include "../types.h"
#include "../psf/psf.h"
#include "../psf/functor.h"

namespace ps
{
namespace church
{

// PS Free Church-encoded

template <typename A>
using StmlFunc = std::function<
        Any(
            std::function<Any(A)>,
            std::function<Any(psf::PSF<Any>)>
        )>;

template <typename A>
struct PSL
{
    StmlFunc<A> runF;
};

} // namespace church
} // namespace ps

#endif // PS_CHURCH_PSL_H

