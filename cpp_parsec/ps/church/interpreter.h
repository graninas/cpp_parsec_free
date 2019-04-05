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
ParseResult<A> runParserF(
        ParserRuntime& runtime,
        const psf::ParserF<A>& psf)
{
    ParserFVisitor<A> visitor(runtime);
    std::visit(visitor, psf.psf);
    return visitor.result;
}

template <typename A>
ParseResult<A> runParserL(ParserRuntime& runtime,
                          const ParserL<A>& psl)
{
    std::function<PRA(A)> pureAny = [](const A& a)
            {
                // cast to any
                return ParseSuccess<Any> { a };
            };

    std::function<PRA(psf::ParserF<PRA>)> g
            = [&](const psf::ParserF<PRA>& psf)
    {
        ParseResult<PRA> r = runParserF<PRA>(runtime, psf);
        if (isLeft(r))
        {
            PRA a = { getError(r) };
            return a;
        }
        else
        {
            PRA a = getParsed<PRA>(r);
            return a;
        }
    };

    try
    {
        PRA anyResult = psl.runF(pureAny, g);
        if (std::holds_alternative<ParseError>(anyResult))
        {
            return { getError(anyResult) };
        }
        else
        {
            Any parsed = getParsed<Any>(anyResult);
            // cast from any
            A a = std::any_cast<A>(parsed);

            ParseSuccess<A> r;
            r.parsed = a;
            return r;
        }
    }
    catch (std::exception ex)
    {
        return { ParseError {ex.what()} };
    }
}

template <typename Single>
ParseResult<Single> parseSingle(
        ParserRuntime& runtime,
        const std::function<bool(char)>& validator,
        const std::function<Single(char)>& converter,
        const std::string& name
        )
{
    std::string_view s = runtime.get_view();
    std::string failedMsg = std::string("Failed to parse ") + name;

    if (s.empty())
    {
        return { ParseError {failedMsg + ": end of imput."} };
    }
    else if (!validator(s.at(0)))
    {
        return { ParseError {failedMsg + ": not a " + name + "."} };
    }

    ParseSuccess<Single> r;
    r.parsed = converter(s.at(0));
    return { r };
}

template <typename Ret>
struct ParserFVisitor
{
    ParserRuntime& _runtime;
    ParseResult<Ret> result;

    ParserFVisitor(ParserRuntime& runtime)
        : _runtime(runtime)
    {
    }

    void operator()(const psf::ParseDigit<Ret>& f)
    {
        auto validator = [](char ch) { return ch >= '0' && ch <= '9'; };
        auto converter = [](char ch) { return uint8_t(ch - '0'); };

        ParseResult<Digit> r = parseSingle<Digit>(_runtime, validator, converter, "digit");

        if (isLeft(r))
            result = { std::get<ParseError>(r) };
        else
        {
            _runtime.advance(1);
            ParseSuccess<Ret> s;
            s.parsed = f.next(getParsed<Digit>(r));
            result = { s };
        }
    }

    void operator()(const psf::ParseUpperCaseChar<Ret>& f)
    {
        auto validator = [](char ch) { return ch >= 'A' && ch <= 'Z'; };
        auto converter = [](char ch) { return ch; };
        ParseResult<Char> r = parseSingle<Char>(_runtime, validator, converter, "upper char");

        if (isLeft(r))
            result = { std::get<ParseError>(r) };
        else
        {
            _runtime.advance(1);
            ParseSuccess<Ret> s;
            s.parsed = f.next(getParsed<Char>(r));
            result = { s };
        }
    }

    void operator()(const psf::ParseLowerCaseChar<Ret>& f)
    {
        auto validator = [](char ch) { return ch >= 'a' && ch <= 'z'; };
        auto converter = [](char ch) { return ch; };
        ParseResult<Char> r = parseSingle<Char>(_runtime, validator, converter, "lower char");

        if (isLeft(r))
            result = { std::get<ParseError>(r) };
        else
        {
            _runtime.advance(1);
            ParseSuccess<Ret> s;
            s.parsed = f.next(getParsed<Char>(r));
            result = { s };
        }
    }
};

} // namespace church
} // namespace ps

#endif // PS_CHURCH_INTERPRETER_H
