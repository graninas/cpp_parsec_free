#ifndef PS_CORE_CHURCH_BIND_H
#define PS_CORE_CHURCH_BIND_H

#include "../types.h"
#include "../methods/adt.h"
#include "../methods/functor.h"
#include "adt.h"
#include "functor.h"

namespace ps
{
namespace core
{
namespace church
{

template <typename A, typename B>
Parser<B> runBind(const Parser<A>& ma,
                  const std::function<Parser<B>(A)>& f)
{
    Parser<B> n;
    n.runF = [=](const std::function<Any(B)>& p,
                 const std::function<Any(ParserMethods<Any, Parser>)>& r)
    {
        auto fst = [=](const A& a)
        {
            Parser<B> internal = f(a);
            return internal.runF(p, r);
        };

        return ma.runF(fst, r);
    };
    return n;
}


} // namespace church
} // namespace core
} // namespace ps

#endif // PS_CORE_CHURCH_BIND_H
