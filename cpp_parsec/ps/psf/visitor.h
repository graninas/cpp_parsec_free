#ifndef PS_PSF_H
#define PS_PSF_H

#include "parserf.h"

namespace ps
{
namespace psf
{

// TODO: remove this, use bind instead.
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
        fb.next = [=](const ParserResult<Char>& d)
        {
            A faResult = fa.next(d);
            B gResult = g(faResult);
            return gResult;
        };
        result.psf = fb;
    }

    void operator()(const ParseLit<A>& fa)
    {
        MapFunc<A, B> g = fTemplate;
        ParseLit<B> fb;
        fb.s = fa.s;
        fb.next = [=](const ParserResult<std::string>& d)
        {
            A faResult = fa.next(d);
            B gResult = g(faResult);
            return gResult;
        };
        result.psf = fb;
    }

    void operator()(const PutSt<A>& fa)
    {
        MapFunc<A, B> g = fTemplate;
        PutSt<B> fb;
        fb.st = fa.st;
        fb.next = [=](const ParserResult<Unit>&)
        {
            A faResult = fa.next(ParserSucceeded<Unit> { unit });
            B gResult = g(faResult);
            return gResult;
        };
        result.psf = fb;
    }

    void operator()(const GetSt<A>& fa)
    {
        MapFunc<A, B> g = fTemplate;
        GetSt<B> fb;
        fb.next = [=](const ParserResult<State>& st)
        {
            A faResult = fa.next(st);
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
