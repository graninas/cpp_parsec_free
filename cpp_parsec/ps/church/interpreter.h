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
    std::cout << "runParserL\n";

    std::function<Any(A)>
            pureAny = [](const A& a) { return a; }; // cast to any

    std::function<Any(psf::ParserF<Any>)> g
            = [&](const psf::ParserF<Any>& psf)
    {
        std::cout << "runParserL -> \\g -> runParserF()\n";
        RunResult<Any> runResult = runParserF<Any>(runtime, psf);

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
        std::cout << "runParserL -> Parsing FAIL " << err.what() << " \n";
        return { ParseError {err.what()} };
    }
    catch (std::exception ex)
    {
        std::cout << "runParserL -> any_cast FAIL " << ex.what() << " \n";
        return { ParseError {ex.what()} };
    }

    std::cout << "runParserL -> success\n";
    return { result };
}

template <typename A>
using LocalParserResult = ps::Either<ParseError, A>;

template <typename Single>
LocalParserResult<Single> parseSingle(
        const std::string_view& s,
        const std::function<bool(char)>& validator,
        const std::function<Single(char)>& converter,
        const std::string& name
        )
{
    std::string failedMsg = std::string("Failed to parse ") + name;

    if (s.empty())
    {
        return { ParseError {failedMsg + ": end of imput."} };
    }
    else if (!validator(s.at(0)))
    {
        return { ParseError {failedMsg + ": not a " + name + "."} };
    }

    return { converter(s.at(0)) };
}

template <typename Ret>
struct ParserFVisitor
{
    ParserRuntime& _runtime;
    RunResult<Ret> result;

    ParserFVisitor(ParserRuntime& runtime)
        : _runtime(runtime)
    {
    }

    void operator()(const psf::ParseDigit<Ret>& f)
    {
        std::string_view s = _runtime.get_view();
        auto validator = [](char ch) { return ch >= '0' && ch <= '9'; };
        auto converter = [](char ch) { return uint8_t(ch - '0'); };
        auto r = parseSingle<Digit>(s, validator, converter, "digit");
        if (isLeft(r))
            result.result = { std::get<ParseError>(r) };
        else
        {
            _runtime.advance(1);
            result.result = f.next(std::get<Digit>(r));
        }
    }

    void operator()(const psf::ParseUpperCaseChar<Ret>& f)
    {
        std::string_view s = _runtime.get_view();
        auto validator = [](char ch) { return ch >= 'A' && ch <= 'Z'; };
        auto converter = [](char ch) { return ch; };
        auto r = parseSingle<Char>(s, validator, converter, "upper char");
        if (isLeft(r))
            result.result = { std::get<ParseError>(r) };
        else
        {
            _runtime.advance(1);
            result.result = f.next(std::get<Char>(r));
        }
    }

    void operator()(const psf::ParseLowerCaseChar<Ret>& f)
    {
        std::string_view s = _runtime.get_view();
        auto validator = [](char ch) { return ch >= 'a' && ch <= 'z'; };
        auto converter = [](char ch) { return ch; };
        auto r = parseSingle<Char>(s, validator, converter, "lower char");
        if (isLeft(r))
            result.result = { std::get<ParseError>(r) };
        else
        {
            _runtime.advance(1);
            result.result = f.next(std::get<Char>(r));
        }
    }
};

} // namespace church
} // namespace ps

#endif // PS_CHURCH_INTERPRETER_H
