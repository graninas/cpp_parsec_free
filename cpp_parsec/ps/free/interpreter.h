#ifndef PS_FREE_INTERPRETER_H
#define PS_FREE_INTERPRETER_H

#include "parserl.h"
#include "../context.h"
#include "../core_parsers.h"

namespace ps
{
namespace free
{

// Forward declaration
template <typename Ret>
struct ParserLVisitor;













template <typename Ret>
ParserResult<Ret> runParser(
        ParserRuntime& runtime,
        const ParserL<Ret>& next,
        Pos start_from)
{
  ParserLVisitor<Ret> visitor(runtime, start_from);
  std::visit(visitor, next.psl);
  return visitor.result;
}














template <typename Ret>
struct ParserADTVisitor
{
    ParserRuntime& _runtime;
    ParserResult<Ret> result;
    Pos _start_from;

    ParserADTVisitor(ParserRuntime& runtime, Pos start_from)
        : _runtime(runtime), _start_from(start_from)
    {
    }

    void operator()(const psf::ParseSymbolCond<ParserL<Ret>>& method)
    {
        ParserResult<Char> r = ps::core::parseSingle<Char>(
          _runtime, _start_from, method.validator, id, method.name);

        if (isLeft(r))
          throw std::runtime_error("ParseSymbolCond failed: " +
            getParseFailed(r).message + " at position " +
            std::to_string(getParseFailed(r).pos));
        else
        {
            ParserSucceeded<Char> succeeded = getParseSucceeded(r);
            ParserL<Ret> rNext = method.next(succeeded.parsed);
            result = runParser<Ret>(_runtime, rNext, succeeded.to);
        }
    }

    void operator()(const psf::ParseLit<ParserL<Ret>>& method)
    {
      ParserResult<std::string> r = ps::core::parseLit<std::string>(_runtime, _start_from, method.s);

      if (isLeft(r))
        throw std::runtime_error("ParseLit failed: " + getParseFailed(r).message +
          " at position " + std::to_string(getParseFailed(r).pos));
      else
      {
        ParserSucceeded<std::string> succeeded = getParseSucceeded(r);
        ParserL<Ret> rNext = method.next(succeeded.parsed);
        result = runParser<Ret>(_runtime, rNext, succeeded.to);
      }
    }

    void operator()(const psf::GetSt<ParserL<Ret>>& method)
    {
        auto rNext = method.next(_runtime.get_state());
        result = runParser<Ret>(_runtime, rNext, _start_from);
    }

    void operator()(const psf::PutSt<ParserL<Ret>>& method)
    {
        _runtime.put_state(method.st);
        auto rNext = method.next(unit);
        result = runParser<Ret>(_runtime, rNext, _start_from);
    }
};

template <typename Ret>
struct ParserLVisitor
{
    ParserRuntime& _runtime;
    ParserResult<Ret> result;
    Pos _start_from;

    ParserLVisitor(ParserRuntime& runtime, Pos start_from)
        : _runtime(runtime), _start_from(start_from)
    {
    }

    void operator()(const PureF<Ret>& p)
    {
        result = ParserSucceeded<Ret> { p.ret, _start_from, p.to };
    }

    void operator()(const FreeF<Ret>& f)
    {
      ParserADTVisitor<Ret> visitor(_runtime, _start_from);
      std::visit(visitor, f.psf.psf);
      result = visitor.result;
    }
};

} // namespace free
} // namespace ps

#endif // PS_FREE_INTERPRETER_H
