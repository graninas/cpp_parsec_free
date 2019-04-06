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

using PRA = ParseResult<Any>;

template <typename A>
using ParserFunc = std::function<
        PRA(
            std::function<PRA(A)>,
            std::function<PRA(psf::ParserF<PRA>)>
        )>;

template <typename A>
struct ParserL
{
    ParserFunc<A> runF;
};

} // namespace church
} // namespace ps

#endif // PS_CHURCH_PSL_H

