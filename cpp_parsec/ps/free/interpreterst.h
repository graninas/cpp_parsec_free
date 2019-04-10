#ifndef PS_FREE_INTERPRETERST_H
#define PS_FREE_INTERPRETERST_H

#include "parserlst.h"
#include "../context.h"
#include "interpreter.h"

namespace ps
{
namespace free
{

// Forward declaration
template <template <typename> class P, typename Ret>
struct ParserLSTVisitor;

template <template <typename> class P, typename Ret>
ParseResult<Ret> runParserT(
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
    void operator()(const psfst::TryP<P, A, ParserLST<P, Ret>>& f)
    {
        State currentSt = _runtime.get_state();
        try
        {
            ParseResult<A> pr1 = runParserL(_runtime, f.parser);
            ParserLST<P, Ret> pr2 = f.next(pr1);
            result = runParserT<P, Ret>(_runtime, pr2);
        }
        catch (const std::runtime_error& err)
        {
            _runtime.put_state(currentSt);
            result = ParseError { err.what() };
        }
    }

    template <typename A>
    void operator()(const psfst::EvalP<P, A, ParserLST<P, Ret>>& f)
    {
        auto pr1 = runParserL(_runtime, f.parser);
        ParserLST<P, Ret> pr2 = f.next(pr1);
        result = runParserT<P, Ret>(_runtime, pr2);
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
