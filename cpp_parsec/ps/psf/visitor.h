#ifndef PS_PSF_H
#define PS_PSF_H

#include "parserf.h"

namespace ps
{
namespace psf
{

template <typename A, typename B>
using MapFunc = std::function<B(A)>;

template <typename A, typename B>
struct ParserFVisitor
{
    MapFunc<A, B> fTemplate;
    ParserF<B> result;

    ParserFVisitor(const MapFunc<A, B>& func)
        : fTemplate(func)
    {}

    void operator()(const ParseDigit<A>& fa)
    {
        MapFunc<A, B> g = fTemplate;
        ParseDigit<B> fb;
        fb.next = [=](const Digit d)
        {
            A faResult = fa.next(d);
            B gResult = g(faResult);
            return gResult;
        };
        result.psf = fb;
    }

    void operator()(const ParseUpperCaseChar<A>& fa)
    {
        MapFunc<A, B> g = fTemplate;
        ParseUpperCaseChar<B> fb;
        fb.next = [=](const Char d)
        {
            A faResult = fa.next(d);
            B gResult = g(faResult);
            return gResult;
        };
        result.psf = fb;
    }
    void operator()(const ParseLowerCaseChar<A>& fa)
    {
        MapFunc<A, B> g = fTemplate;
        ParseLowerCaseChar<B> fb;
        fb.next = [=](const Char d)
        {
            A faResult = fa.next(d);
            B gResult = g(faResult);
            return gResult;
        };
        result.psf = fb;
    }
};

template <typename A, typename B>
ParserF<B> fmap(const MapFunc<A, B>& f,
                const ParserF<A>& method)
{
    ParserFVisitor<A, B> visitor(f);
    std::visit(visitor, method.psf);
    return visitor.result;
}


} // namespace psf
} // namespace ps

#endif // PS_PSF_H
