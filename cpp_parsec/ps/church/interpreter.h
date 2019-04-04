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
        const psf::ParserF<A>& psf,
        size_t position)
{
    ParserFVisitor<A> visitor(runtime, position);
    std::visit(visitor, psf.psf);
    return visitor.result;
}

template <typename A>
RunResult<A> runParserL(ParserRuntime& runtime,
                        const ParserL<A>& psl,
                        size_t position)
{
    std::cout << "runParserL\n";

    std::function<Any(A)>
            pureAny = [](const A& a) { return a; }; // cast to any

    std::function<Any(psf::ParserF<Any>)> g
            = [&, position](const psf::ParserF<Any>& psf)
    {
        std::cout << "runParserL -> \\g -> runParserF()\n";
        RunResult<Any> runResult = runParserF<Any>(runtime, psf, position);

        if (isLeft(runResult.result))
        {
            std::cout << "runParserL -> \\g -> isLeft\n";
            ParseError pe = std::get<ParseError>(runResult.result);
            throw std::runtime_error(pe.message);
        }
        std::cout << "runParserL -> \\g -> isRight\n";
        return std::get<Any>(runResult.result);
    };

    A result;
    try
    {
        std::cout << "runParserL -> psl.runF(pureAny, g)\n";
        Any anyResult = psl.runF(pureAny, g);

        std::cout << "runParserL -> any_cast()\n";
        result = std::any_cast<A>(anyResult); // cast from any

        std::cout << "runParserL -> any_cast OK\n";
    }
    catch (std::runtime_error err)
    {
        std::cout << "runParserL -> any_cast FAIL\n";
        return { ParseError {err.what()}, position };
    }

    std::cout << "runParserL -> success\n";
    return { result, position };
}

template <typename Single>
RunResult<Single> parseSingle(
        const std::string_view& s,
        const size_t position,
        const std::function<bool(char)>& validator,
        const std::function<Single(char)>& converter,
        const std::string& name
        )
{
    std::string failedMsg = std::string("Failed to parse ") + name;

    if (s.empty())
    {
        return { ParseError {failedMsg + ": end of imput."}, position };
    }
    else if (!validator(s.at(0)))
    {
        return { ParseError {failedMsg + ": not a " + name + "."}, position };
    }

    return { converter(s.at(0)), position + 1 };
}

template <typename Ret>
struct ParserFVisitor
{
    ParserRuntime& _runtime;
    size_t _position;

    ParserFVisitor(ParserRuntime& runtime,
                   size_t position)
        : _runtime(runtime)
        , _position(position)
    {
    }

    RunResult<Ret> result;

    void operator()(const psf::ParseDigit<Ret>&)
    {
        std::string_view s = _runtime.get_view();
        auto validator = [](char ch) { return ch >= '0' && ch <= '9'; };
        auto converter = [](char ch) { return uint8_t(ch - '0'); };
        auto r = parseSingle<Ret>(s, _position, validator, converter, "digit");
        result.result = r.result;
        result.position = r.position;
    }

    void operator()(const psf::ParseUpperCaseChar<Ret>&)
    {
        std::string_view s = _runtime.get_view();
        auto validator = [](char ch) { return ch >= 'A' && ch <= 'Z'; };
        auto converter = [](char ch) { return ch; };
        auto r = parseSingle<Ret>(s, _position, validator, converter, "upper char");
        result.result = r.result;
        result.position = r.position;
    }

    void operator()(const psf::ParseLowerCaseChar<Ret>&)
    {
        std::string_view s = _runtime.get_view();
        auto validator = [](char ch) { return ch >= 'a' && ch <= 'z'; };
        auto converter = [](char ch) { return ch; };
        auto r = parseSingle<Ret>(s, _position, validator, converter, "lower char");
        result.result = r.result;
        result.position = r.position;
    }
};

} // namespace church
} // namespace ps

#endif // PS_CHURCH_INTERPRETER_H
