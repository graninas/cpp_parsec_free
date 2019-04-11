#ifndef PS_CHURCH_INTERPRETER_H
#define PS_CHURCH_INTERPRETER_H

#include "parserl.h"
#include "../types.h"
#include "../context.h"
#include <cstring>

namespace ps
{
namespace church
{

// Forward declaration
template <typename Ret>
struct ParserFVisitor;

template <typename A>
ParserResult<A> runParserF(
        ParserRuntime& runtime,
        const psf::ParserF<A>& psf)
{
    ParserFVisitor<A> visitor(runtime);
    std::visit(visitor, psf.psf);
    return visitor.result;
}

template <typename A>
ParserResult<A> runParserL(ParserRuntime& runtime,
                          const ParserL<A>& psl)
{
    std::function<PRA(A)> pureAny = [](const A& a)
            {
                // cast to any
                return ParserSucceeded<Any> { a };
            };

    std::function<PRA(psf::ParserF<PRA>)> g
            = [&](const psf::ParserF<PRA>& psf)
    {
        ParserResult<PRA> r = runParserF<PRA>(runtime, psf);
        if (isLeft(r))
        {
            return PRA { getError(r) };
        }
        else
        {
            return PRA { getParsed<PRA>(r) };
        }
    };

    try
    {
        PRA anyResult = psl.runF(pureAny, g);
        if (std::holds_alternative<ParserFailed>(anyResult))
        {
            return { getError(anyResult) };
        }
        else
        {
            Any parsed = getParsed<Any>(anyResult);
            // cast from any
            A a = std::any_cast<A>(parsed);
            return ParserSucceeded<A> { a };
        }
    }
    catch (std::exception ex)
    {
        return { ParserFailed {ex.what()} };
    }
}

template <typename Ret>
struct ParserFVisitor
{
    ParserRuntime& _runtime;
    ParserResult<Ret> result;

    ParserFVisitor(ParserRuntime& runtime)
        : _runtime(runtime)
    {
    }

    void operator()(const psf::ParseSymbolCond<Ret>& f)
    {
        ParserResult<Char> r = parseSingle<Char>(_runtime, f.validator, id, f.name);

        if (isLeft(r))
            result = { std::get<ParserFailed>(r) };
        else
        {
            _runtime.advance(1);
            ParserSucceeded<Ret> s;
            s.parsed = f.next(getParsed<Char>(r));
            result = { s };
        }
    }
};

} // namespace church
} // namespace ps

#endif // PS_CHURCH_INTERPRETER_H
