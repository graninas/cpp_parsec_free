#ifndef PS_VSITORST_H
#define PS_VSITORST_H

#include "parserfst.h"

namespace ps
{
namespace psfst
{

template <typename A, typename B>
using MapFuncST = std::function<B(A)>;

template <typename A, typename B>
struct ParserFSTVisitor
{
    MapFuncST<A, B> fTemplate;
    ParserFST<B> result;

    ParserFSTVisitor(const MapFuncST<A, B>& func)
        : fTemplate(func)
    {}

    void operator()(const SafePA<A>& fa)
    {
        MapFuncST<A, B> g = fTemplate;
        SafePA<B> fb;
        fb.parser = fa.parser;
        fb.next = [=](const ParserResult<Any>& result)
        {
            return g(fa.next(result));
        };
        result.psfst = fb;
    }

    void operator()(const TryPA<A>& fa)
    {
        MapFuncST<A, B> g = fTemplate;
        TryPA<B> fb;
        fb.parser = fa.parser;
        fb.next = [=](const ParserResult<Any>& result)
        {
            return g(fa.next(result));
        };
        result.psfst = fb;
    }

    void operator()(const EvalPA<A>& fa)
    {
        MapFuncST<A, B> g = fTemplate;
        EvalPA<B> fb;
        fb.parser = fa.parser;
        fb.next = [=](const ParserResult<Any>& result)
        {
            return g(fa.next(result));
        };
        result.psfst = fb;
    }
};

template <typename A, typename B>
ParserFST<B> fmap(
        const MapFuncST<A, B>& f,
        const ParserFST<A>& method)
{
    ParserFSTVisitor<A, B> visitor(f);
    std::visit(visitor, method.psfst);
    return visitor.result;
}

} // namespace psfst
} // namespace ps

#endif // PS_VSITORST_H
