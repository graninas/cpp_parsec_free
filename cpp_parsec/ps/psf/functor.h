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
struct ParserFFunctorVisitor
{
    MapFunc<A,B> fTemplate;
    ParserF<B> result;

    ParserFFunctorVisitor(const MapFunc<A,B>& func)
        : fTemplate(func)
    {}

    void operator()(const ParseDigit<A>& fa)
    {
        MapFunc<A,B> g = fTemplate;
        ParseDigit<B> fb;
        fb.next = [=](const Digit d)
        {
            return g(fa.next(d));
        };
        result.psf = fb;
    }

    void operator()(const ParseUpperCaseChar<A>& fa)
    {
        MapFunc<A,B> g = fTemplate;
        ParseUpperCaseChar<B> fb;
        fb.next = [=](const Digit d)
        {
            return g(fa.next(d));
        };
        result.psf = fb;
    }
    void operator()(const ParseLowerCaseChar<A>& fa)
    {
        MapFunc<A,B> g = fTemplate;
        ParseLowerCaseChar<B> fb;
        fb.next = [=](const Digit d)
        {
            return g(fa.next(d));
        };
        result.psf = fb;
    }
};

template <typename A, typename B>
ParserF<B> fmap(const MapFunc<A, B>& f,
             const ParserF<A>& method)
{
    ParserFFunctorVisitor<A, B> visitor(f);
    std::visit(visitor, method.psf);
    return visitor.result;
}


} // namespace psf
} // namespace ps

#endif // PS_PSF_FUNCTOR_H
