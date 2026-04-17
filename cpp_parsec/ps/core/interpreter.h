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
        ParserResult<Char> r = parseSingle<Unit>(
          _runtime, _start_from, method.validator, method.name);

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

    void operator()(const ParseMany<ParserL<Ret>>& method)
    {
      if (method.raw_parser == nullptr)
      {
        _runtime.push_message("ParseMany: raw_parser is null.");
        result = ParserFailed { "Internal error: null parser in ParseMany.", _start_from };
        return;
      }

      _runtime.push_message("Current src view: '" + std::string(_runtime.get_view())+ "'");

      // Create a temporary runtime to run the raw parser, so that we don't modify the original runtime's state and messages during the loop.
      // N.B. For now, the source string is copied in the constructor of ParserRuntime, so this is not a problem. If we later change it to use a shared_ptr or something similar to avoid copying the source string, we might need to make sure that the temporary runtime shares the same source string as the original runtime, so that we don't end up with two different source strings in memory. For now, we just keep it simple and copy the runtime.
      ParserRuntime tempRuntime = _runtime;

      std::list<Any> acc;
      Pos currentPos = _start_from;
      int iteration = 0;

      _runtime.push_message("ParseMany: starting first runParser. Current position: " + std::to_string(currentPos) + ".");
      ParserResult<Any> r = runParser<Any>(tempRuntime, *method.raw_parser, currentPos);
      _runtime.push_message("ParseMany: entering loop.");

      while (isRight(r))
      {
        ParserSucceeded<Any> succeeded = getParseSucceeded(r);
        acc.push_back(succeeded.parsed);    // Result will be converted when fmapped

        currentPos = succeeded.to;
        _runtime.push_message("ParseMany: current position after parsing: " + std::to_string(currentPos) + ". Parsed value added to accumulator, size now: " + std::to_string(acc.size()) + ".");
        iteration++;
        r = runParser<Any>(tempRuntime, *method.raw_parser, currentPos);

        if (iteration > _runtime.get_many_combinator_threshold())   // Safety check to prevent infinite loops, in case of a bug in the raw parser or something like that. In a real implementation we might want to handle this differently, maybe by throwing an exception or something like that.
        {
          _runtime.push_message("ParseMany: safety check triggered, breaking loop after " + std::to_string(iteration) + " iterations.");
          break;
        }
      }
      _runtime.push_message("ParseMany: loop ended after " + std::to_string(iteration) + " iterations.");

      auto messages = tempRuntime.get_messages();   // Get messages from the temporary runtime and push them to the original runtime, so that we have the messages from all iterations of the loop.
      for (const auto& msg : messages)
      {
          _runtime.push_message(msg);
      }

      result = runParser<Ret>(_runtime, method.next(acc), currentPos);
      _runtime.push_message("ParseMany: finished.");
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
      result = ParserSucceeded<Ret>{p.ret, _start_from, _start_from};
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
