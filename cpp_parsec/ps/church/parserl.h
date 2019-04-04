#ifndef PS_CHURCH_PSL_H
#define PS_CHURCH_PSL_H

#include "../types.h"
#include "../psf/parserf.h"
#include "../psf/visitor.h"

namespace ps
{
namespace church
{

// PS Free Church-encoded

template <typename A>
using ParserFunc= std::function<
        Any(
            std::function<Any(A)>,
            std::function<Any(psf::ParserF<Any>)>
        )>;

template <typename A>
struct ParserL
{
    ParserFunc<A> runF;
};

} // namespace church
} // namespace ps

#endif // PS_CHURCH_PSL_H

