#ifndef PS_VSITORST_H
#define PS_VSITORST_H

#include "parserfst.h"

namespace ps
{
namespace psfst
{

template <typename A, typename B>
using MapFuncST = std::function<B(A)>;

template <template <typename> class P,
          typename A,
          typename B>
struct ParserFSTVisitor
{
    MapFuncST<A, B> fTemplate;
    ParserFST<P, B> result;

    ParserFSTVisitor(const MapFuncST<A, B>& func)
        : fTemplate(func)
    {}

    void operator()(const TryPA<P, A>& fa)
    {
        MapFuncST<A, B> g = fTemplate;
        TryPA<P, B> fb;
        fb.parser = fa.parser;
        fb.next = [=](const ParseResult<Any>& result)
        {
            return g(fa.next(result));
        };
        result.psfst = fb;
    }
};

template <template <typename> class P,
          typename A,
          typename B>
ParserFST<P, B> fmap(
        const MapFuncST<A, B>& f,
        const ParserFST<P, A>& method)
{
    ParserFSTVisitor<P, A, B> visitor(f);
    std::visit(visitor, method.psfst);
    return visitor.result;
}

} // namespace psfst
} // namespace ps

#endif // PS_VSITORST_H
