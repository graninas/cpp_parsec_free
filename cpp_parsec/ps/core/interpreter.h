#ifndef PS_CORE_INTERPRETER_H
#define PS_CORE_INTERPRETER_H

#include "runtime.h"
#include "raw_parsers.h"
#include "parser/adt.h"

namespace ps
{
namespace core
{

// Forward declaration
template <typename Ret>
struct InterpretingVisitor;


template <typename Ret>
ParserResult<Ret> runParser(
        ParserRuntime& runtime,
        const ParserL<Ret>& next,
        Pos start_from)
{
  InterpretingVisitor<Ret> visitor(runtime, start_from);
  std::visit(visitor, next.psl);
  return visitor.result;
}


template <typename Ret>
struct InterpretingADTVisitor
{
    ParserRuntime& _runtime;
    ParserResult<Ret> result;
    Pos _start_from;

    InterpretingADTVisitor(ParserRuntime& runtime, Pos start_from)
        : _runtime(runtime), _start_from(start_from)
    {
    }

    void operator()(const ParseSymbolCond<ParserL<Ret>>& method)
    {
        ParserResult<Char> r = parseSingle<Char>(
          _runtime, _start_from, method.validator, id, method.name);

        if (isLeft(r))
        {
          ParserFailed failed = getParseFailed(r);
          result = ParserFailed { failed.message, failed.pos };
        }
        else
        {
            ParserSucceeded<Char> succeeded = getParseSucceeded(r);
            ParserL<Ret> rNext = method.next(succeeded.parsed);
            result = runParser<Ret>(_runtime, rNext, succeeded.to);
        }
    }

    void operator()(const ParseLit<ParserL<Ret>>& method)
    {
      ParserResult<std::string> r = parseLit<std::string>(_runtime, _start_from, method.s);

      if (isLeft(r))
      {
        ParserFailed failed = getParseFailed(r);
        result = ParserFailed { failed.message, failed.pos };
      }
      else
      {
        ParserSucceeded<std::string> succeeded = getParseSucceeded(r);
        ParserL<Ret> rNext = method.next(succeeded.parsed);
        result = runParser<Ret>(_runtime, rNext, succeeded.to);
      }
    }

    void operator()(const GetSt<ParserL<Ret>>& method)
    {
        auto rNext = method.next(_runtime.get_state());
        result = runParser<Ret>(_runtime, rNext, _start_from);
    }

    void operator()(const PutSt<ParserL<Ret>>& method)
    {
        _runtime.put_state(method.st);
        auto rNext = method.next(unit);
        result = runParser<Ret>(_runtime, rNext, _start_from);
    }
};

template <typename Ret>
struct InterpretingVisitor
{
    ParserRuntime& _runtime;
    ParserResult<Ret> result;
    Pos _start_from;

    InterpretingVisitor(ParserRuntime& runtime, Pos start_from)
        : _runtime(runtime), _start_from(start_from)
    {
    }

    void operator()(const PureF<Ret>& p)
    {
        result = ParserSucceeded<Ret> { p.ret, _start_from, p.to };
    }

    void operator()(const FreeF<Ret>& f)
    {
      InterpretingADTVisitor<Ret> visitor(_runtime, _start_from);
      std::visit(visitor, f.psf.psf);
      result = visitor.result;
    }
};

} // namespace core
} // namespace ps

#endif // PS_CORE_INTERPRETER_H
