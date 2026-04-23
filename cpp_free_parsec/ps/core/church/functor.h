#ifndef PS_CORE_CHURCH_FUNCTOR_H
#define PS_CORE_CHURCH_FUNCTOR_H

#include "../types.h"
#include "../methods/adt.h"
#include "../methods/functor.h"
#include "adt.h"

namespace ps
{
namespace core
{
namespace church
{

template <typename A, typename B>
Parser<B> runFMap(
        const std::function<B(A)>& f,
        const Parser<A>& church)
{
    Parser<B> n;

    n.debugInfo = church.debugInfo + " (fmap)";
    n.runF = [=](
            const std::function<Any(B)>& p,
            const std::function<Any(ParserMethods<Any, Parser>)>& s)
    {
        auto composed = [=](const A& a)
        {
            return p(f(a));
        };

        return church.runF(composed, s);
    };

    return  n;
}

} // namespace church
} // namespace core
} // namespace ps

#endif // PS_CORE_CHURCH_FUNCTOR_H
