#ifndef PS_FREE_INTERPRETER_H
#define PS_FREE_INTERPRETER_H

#include "parserl.h"
#include "../context.h"

namespace ps
{
namespace free
{

// Forward declaration
template <typename Ret>
struct ParserLVisitor;

template <typename Ret>
ParseResult<Ret> runParserL(
        ParserRuntime& runtime,
        const ParserL<Ret>& psl)
{
    ParserLVisitor<Ret> visitor(runtime);
    std::visit(visitor, psl.psl);
    return visitor.result;
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

    void operator()(const psf::ParseSymbolCond<ParserL<Ret>>& f)
    {
        ParseResult<Char> r = parseSingle<Char>(_runtime, f.validator, id, f.name);

        if (isLeft(r))
            result = { std::get<ParseError>(r) };
        else
        {
            _runtime.advance(1);

            Char ch = std::get<ParseSuccess<Char>>(r).parsed;
            auto rNext = f.next(ch);
            result = runParserL<Ret>(_runtime, rNext);
        }
    }
};

template <typename Ret>
struct ParserLVisitor
{
    ParserRuntime& _runtime;
    ParseResult<Ret> result;

    ParserLVisitor(ParserRuntime& runtime)
        : _runtime(runtime)
    {
    }

    void operator()(const PureF<Ret>& p)
    {
        result = ParseSuccess<Ret> { p.ret };
    }

    void operator()(const FreeF<Ret>& f)
    {
        ParserFVisitor<Ret> visitor(_runtime);
        std::visit(visitor, f.psf.psf);
        result = visitor.result;
    }
};

} // namespace free
} // namespace ps

#endif // PS_FREE_INTERPRETER_H
