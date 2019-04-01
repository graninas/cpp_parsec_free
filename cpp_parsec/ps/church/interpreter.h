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
RunResult<A> runParserL(ParserRuntime& runtime,
                        const ParserL<A>& psl)
{
    std::function<Any(A)>
            pureAny = [](const A& a) { return a; }; // cast to any

    std::function<Any(psf::ParserF<Any>)> g
            = [&](const psf::ParserF<Any>& psf)
    {
        RunResult<Any> runResult = runParserF<Any>(runtime, psf);
        if (isLeft(runResult.result))
        {
            ParseError pe = std::get<ParseError>(runResult.result);
            throw std::runtime_error(pe.message);
        }
        return std::get<Any>(runResult.result);
    };

    A result;
    try
    {
        Any anyResult = psl.runF(pureAny, g);
        result = std::any_cast<A>(anyResult); // cast from any
    }
    catch(std::runtime_error err)
    {
        return RunResult<A> { ParseError {err.what()} };
    }
    return RunResult<A> { result };
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

    void operator()(const psf::ParseDigit<Ret>& f)
    {
        std::string_view s = _runtime.get_view();

        if (s.empty())
        {
            result = { ParseError {"Failed to parse digit: end of imput."} };
            return;
        }
        else if (s.at(0) >= '0' && s.at(0) <= '9')
        {
            result = { ParseError {"Failed to parse digit: not a digit."} };
            return;
        }

        uint8_t digit = s.at(0) - '0';
        result = { digit };
    }
};

} // namespace church
} // namespace ps

#endif // PS_CHURCH_INTERPRETER_H
