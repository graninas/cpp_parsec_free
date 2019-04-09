#ifndef PS_VSITORST_H
#define PS_VSITORST_H

#include "parserfst.h"

namespace ps
{
namespace psfst
{

template <typename A, typename B>
using MapFunc = std::function<B(A)>;

template <typename A, typename B>
struct ParserFSTVisitor
{
    MapFunc<A, B> fTemplate;
    ParserFST<B> result;

    ParserFSTVisitor(const MapFunc<A, B>& func)
        : fTemplate(func)
    {}

    void operator()(const PutSt<A>& fa)
    {
        MapFunc<A, B> g = fTemplate;
        PutSt<B> fb;
        fb.st = fa.st;
        fb.next = [=](const Unit&)
        {
            A faResult = fa.next(unit);
            B gResult = g(faResult);
            return gResult;
        };
        result.psfst = fb;
    }

    void operator()(const GetSt<A>& fa)
    {
        MapFunc<A, B> g = fTemplate;
        PutSt<B> fb;
        fb.next = [=](const State& st)
        {
            A faResult = fa.next(st);
            B gResult = g(faResult);
            return gResult;
        };
        result.psfst = fb;
    }
};

template <typename A, typename B>
ParserFST<B> fmap(const MapFunc<A, B>& f,
                const ParserFST<A>& method)
{
    ParserFSTVisitor<A, B> visitor(f);
    std::visit(visitor, method.psfst);
    return visitor.result;
}

} // namespace psfst
} // namespace ps

#endif // PS_VSITORST_H
