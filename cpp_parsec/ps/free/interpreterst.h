#ifndef PS_FREE_INTERPRETERST_H
#define PS_FREE_INTERPRETERST_H

#include "parserlst.h"
#include "../context.h"

namespace ps
{
namespace free
{

// Forward declaration
template <template <typename> class P, typename Ret>
struct ParserLSTVisitor;

template <template <typename> class P, typename Ret>
ParseResult<Ret> runParserLST(
        ParserRuntime& runtime,
        const ParserLST<P, Ret>& pslst)
{
    ParserLSTVisitor<P, Ret> visitor(runtime);
    std::visit(visitor, pslst.pslst);
    return visitor.result;
}

template <template <typename> class P, typename Ret>
struct ParserFSTVisitor
{
    ParserRuntime& _runtime;
    ParseResult<Ret> result;

    ParserFSTVisitor(ParserRuntime& runtime)
        : _runtime(runtime)
    {
    }

    template <typename A>
    void operator()(const psfst::TryP<A, P, ParserLST<P, Ret>>& f)
    {
        // TODO
        int i = 10;
//        auto tvarId = _runtime.newId();
//        TVarHandle tvarHandle { _runtime.getUStamp(), f.val, true };
//        _runtime.addTVarHandle(tvarId, tvarHandle);
//        TVarAny tvar { f.name, tvarId };
//        result = runSTML<Ret, StmlVisitor>(_runtime, f.next(tvar));
    }
};

template <template <typename> class P, typename Ret>
struct ParserLSTVisitor
{
    ParserRuntime& _runtime;
    ParseResult<Ret> result;

    ParserLSTVisitor(ParserRuntime& runtime)
        : _runtime(runtime)
    {
    }

    void operator()(const PureFST<P, Ret>& p)
    {
        result = ParseSuccess<Ret> { p.ret };
    }

    void operator()(const FreeFST<P, Ret>& f)
    {
        ParserFSTVisitor<P, Ret> visitor(_runtime);
        std::visit(visitor, f.psfst.psfst);
        result = visitor.result;
    }
};

} // namespace free
} // namespace ps

#endif // PS_FREE_INTERPRETERST_H
