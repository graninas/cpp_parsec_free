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
        size_t position,
        const psf::ParserF<A>& psf)
{
    ParserFVisitor<A> visitor(runtime, position);
    std::visit(visitor, psf.psf);
    return visitor.result;
}

template <typename A>
ParseResult<A> runParserL(ParserRuntime& runtime,
                          size_t position,
                          const ParserL<A>& psl)
{
    std::function<PRA(A)> pureAny = [=](const A& a)
            {
                // cast to any
                return ParseSuccess<Any> { a, position };
            };

    std::function<PRA(psf::ParserF<PRA>)> g
            = [&, position](const psf::ParserF<PRA>& psf)
    {
        ParseResult<PRA> r = runParserF<PRA>(runtime, position, psf);
        if (isLeft(r))
        {
            return PRA { getError(r) };
        }
        else
        {
            ParseSuccess<PRA> success = std::get<ParseSuccess<PRA>>(r);
            return success.parsed;  // next pos?
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
            ParseSuccess<Any> successAny = std::get<ParseSuccess<Any>>(anyResult);
            A a = std::any_cast<A>(successAny.parsed); // cast from any
            ParseSuccess<A> success { a, success.next_position };
            return success;
        }
    }
    catch (std::exception ex)
    {
        return { ParseError { ex.what(), position } };
    }
}

template <typename Single>
ParseResult<Single> parseSingle(
        ParserRuntime& runtime,
        size_t position,
        const std::function<bool(char)>& validator,
        const std::function<Single(char)>& converter,
        const std::string& name
        )
{
    std::string_view s = runtime.get_view(position);
    std::string failedMsg = std::string("Failed to parse ") + name;

    if (s.empty())
    {
        return { ParseError { failedMsg + ": end of imput.", position } };
    }
    else if (!validator(s.at(0)))
    {
        return { ParseError { failedMsg + ": not a " + name + ".", position } };
    }

    return ParseSuccess<Single> { converter(s.at(0)), position + 1 };
}

template <typename Ret>
struct ParserFVisitor
{
    ParserRuntime& _runtime;
    size_t _position;
    ParseResult<Ret> result;

    ParserFVisitor(ParserRuntime& runtime, size_t position)
        : _runtime(runtime)
        , _position(position)
    {
    }

    void operator()(const psf::ParseSymbolCond<Ret>& f)
    {
        ParseResult<Char> r = parseSingle<Char>(_runtime, _position, f.validator, id, f.name);

        if (isLeft(r))
            result = { std::get<ParseError>(r) };
        else
        {
            ParseSuccess<Char> success = std::get<ParseSuccess<Char>>(r);
            auto next = f.next(success.parsed, success.next_position);
            result = { ParseSuccess<Ret> { next, success.next_position } };
        }
    }

    void operator()(const psf::FailWith<Ret>& f)
    {
        // TODO: does not call f.next?
        result = ParseError { f.message, _position };
    }
};

} // namespace church
} // namespace ps

#endif // PS_CHURCH_INTERPRETER_H
