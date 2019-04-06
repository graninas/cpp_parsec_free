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

    void operator()(const ParseSymbolCond<A>& fa)
    {
        MapFunc<A, B> g = fTemplate;
        ParseSymbolCond<B> fb;
        fb.name = fa.name;
        fb.validator = fa.validator;
        fb.next = [=](const Char d)
        {
            A faResult = fa.next(d);
            B gResult = g(faResult);
            return gResult;
        };
        result.psf = fb;
    }

    void operator()(const FailWith<A>& fa)
    {
        MapFunc<A, B> g = fTemplate;
        FailWith<B> fb;
        fb.message = fa.message;
        fb.next = [=](const Any& a)
        {
            A faResult = fa.next(a);
            B gResult = g(faResult);
            return gResult;
        };
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
