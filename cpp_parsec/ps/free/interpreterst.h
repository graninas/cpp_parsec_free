#ifndef PS_FREE_INTERPRETERST_H
#define PS_FREE_INTERPRETERST_H

#include "parserlst.h"
#include "../context.h"
#include "interpreter.h"
#include "bind.h"

namespace ps
{
namespace free
{

// Forward declaration
template <typename Ret>
struct ParserLSTVisitor;

template <typename Ret>
ParserResult<Ret> runParserT(
        ParserRuntime& runtime,
        const ParserLST<Ret>& pslst)
{
    ParserLSTVisitor<Ret> visitor(runtime);
    std::visit(visitor, pslst.pslst);
    return visitor.result;
}

template <typename Ret>
struct ParserFSTVisitor
{
    ParserRuntime& _runtime;
    ParserResult<Ret> result;

    ParserFSTVisitor(ParserRuntime& runtime)
        : _runtime(runtime)
    {
    }

    template <typename A>
    void operator()(const psfst::SafeP<A, ParserLST<Ret>>& f)
    {
        State currentSt = _runtime.get_state();
        try
        {
            ParserResult<A> pr1 = runParserL(_runtime, f.parser);
            ParserLST<Ret> pr2 = f.next(pr1);
            result = runParserT<Ret>(_runtime, pr2);
        }
        catch (const std::runtime_error& err)
        {
            _runtime.put_state(currentSt);

            ParserLST<Ret> pr2 = f.next(ParserFailed { err.what() });
            result = runParserT<Ret>(_runtime, pr2);
        }
    }

    template <typename A>
    void operator()(const psfst::TryP<A, ParserLST<Ret>>& f)
    {
        State currentSt = _runtime.get_state();
        try
        {
            ParserResult<A> pr1 = runParserL(_runtime, f.parser);
            ParserLST<Ret> pr2 = f.next(pr1);
            result = runParserT<Ret>(_runtime, pr2);
        }
        catch (const std::runtime_error& err)
        {
            _runtime.put_state(currentSt);

//            ParserLST<Ret> pr2 = f.next(ParserFailed { err.what() });
//            result = runParserT<Ret>(_runtime, pr2);
            throw std::runtime_error(err.what());
        }
    }

    template <typename A>
    void operator()(const psfst::EvalP<A, ParserLST<Ret>>& f)
    {
        ParserResult<A> pr1 = runParserL(_runtime, f.parser);
        ParserLST<Ret> pr2 = f.next(pr1);
        result = runParserT<Ret>(_runtime, pr2);
    }
};

template <typename Ret>
struct ParserLSTVisitor
{
    ParserRuntime& _runtime;
    ParserResult<Ret> result;

    ParserLSTVisitor(ParserRuntime& runtime)
        : _runtime(runtime)
    {
    }

    void operator()(const PureFST<Ret>& p)
    {
        result = ParserSucceeded<Ret> { p.ret };
    }

    void operator()(const FreeFST<Ret>& f)
    {
        ParserFSTVisitor<Ret> visitor(_runtime);
        std::visit(visitor, f.psfst.psfst);
        result = visitor.result;
    }
};

} // namespace free
} // namespace ps

#endif // PS_FREE_INTERPRETERST_H
