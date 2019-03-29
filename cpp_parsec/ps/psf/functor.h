#ifndef PS_PSF_FUNCTOR_H
#define PS_PSF_FUNCTOR_H

#include "parserf.h"

namespace ps
{
namespace psf
{

template <typename A, typename B>
using MapFunc = std::function<B(A)>;

template <typename A, typename B>
struct PSFFunctorVisitor
{
    MapFunc<A,B> fTemplate;
    ParserF<B> result;

    PSFFunctorVisitor(const MapFunc<A,B>& func)
        : fTemplate(func)
    {}

    void operator()(const ParseDigit<A>& fa)
    {
        MapFunc<A,B> g = fTemplate;
        ParseDigit<B> fb;
        fb.ch = fa.ch;

        // This is probably not needed.
        fb.next = [=](const Digit& dAny)
        {
            return g(fa.next(dAny));
        };
        result.psf = fb;
    }

    void operator()(const ParseChar<A>& fa)
    {
        MapFunc<A,B> g = fTemplate;
        ParseChar<B> fb;
        fb.ch = fa.ch;

        // This is probably not needed.
        fb.next = [=](const S& sAny)
        {
            return g(fa.next(sAny));
        };
        result.psf = fb;
    }
};

template <typename A, typename B>
ParserF<B> fmap(const MapFunc<A, B>& f,
             const ParserF<A>& method)
{
    PSFFunctorVisitor<A, B> visitor(f);
    std::visit(visitor, method.psf);
    return visitor.result;
}

} // namespace psf
} // namespace ps

#endif // PS_PSF_FUNCTOR_H
