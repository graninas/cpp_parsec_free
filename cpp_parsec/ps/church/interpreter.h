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
RunResult<A> runParserF(
        ParserRuntime& runtime,
        const psf::ParserF<A>& psf)
{
    ParserFVisitor<A> visitor(runtime);
    std::visit(visitor, psf.psf);
    return visitor.result;
}

template <typename A>
RunResult<A> runPSL(ParserRuntime& runtime,
                     const ParserL<A>& psl)
{
    std::function<Any(A)>
            pureAny = [](const A& a) { return a; }; // cast to any

    std::function<Any(psf::ParserF<Any>)> g
            = [&](const psf::ParserF<Any>& psf)
    {
        auto runnerResult = runPSF(runtime, psf);
        if (runnerResult.retry)
        {
            throw std::runtime_error("Retry");
        }
        if (!runnerResult.result.has_value())
        {
            throw std::runtime_error("No result.");
        }
        return runnerResult.result.value();
    };

    A result;
    try
    {
        Any anyResult = psl.runF(pureAny, g);
        result = std::any_cast<A>(anyResult);
    }
    catch(std::runtime_error err)
    {
        if (strcmp(err.what(), "Retry") == 0)
        {
            return RunResult<A> { true, std::nullopt };
        }
        throw err;
    }
    return RunResult<A> { false, result };
}

template <typename Ret>
struct ParserFVisitor
{
    ParserRuntime& _runtime;

    ParserFVisitor(ParserRuntime& runtime)
        : _runtime(runtime)
    {
    }

    RunResult<Ret> result;

    template <typename A>
    void operator()(const psf::ParseDigit<A, Ret>& f)
    {
        if (_runtime.empty())
        {
            result = {ParseError {"Failed to parse digit: end of imput."}};
            return;
        }
    }
};

} // namespace church
} // namespace ps

#endif // PS_CHURCH_INTERPRETER_H
