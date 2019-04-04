#ifndef PS_CHURCH_FUNCTOR_H
#define PS_CHURCH_FUNCTOR_H

#include "parserl.h"

namespace ps
{
namespace church
{

//template <typename A, typename B>
//PSL<B> fmap(
//        const std::function<B(A)>& f,
//        const PSL<A>& church)
//{
//    PSL<B> n;

//    n.runF = [=](
//            const std::function<Any(B)>& p,
//            const std::function<Any(psf::PSF<Any>)>& s)
//    {
//        auto composed = [=](const A& a)
//        {
//            return p(f(a));
//        };

//        return church.runF(composed, s);
//    };

//    return  n;
//}

} // namespace church
} // namespace ps

#endif // PS_CHURCH_FUNCTOR_H
