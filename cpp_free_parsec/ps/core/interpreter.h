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
        const Parser<Ret>& next,
        Pos startFrom)
{
  InterpretingVisitor<Ret> visitor(runtime, startFrom);

  try
  {
      std::visit(visitor, next.psl);
  }
  catch (const std::exception& e)
  {
      auto result = visitor.result;
      if (isLeft(result))
      {
        ParserFailed failed = getParseFailed(result);
        return ParserFailed{"Exception in runParser: " + std::string(e.what()) + ". Original error message: " + failed.message, failed.at};
      }
      else
      {
        // Should not happen
        ParserSucceeded<Ret> succeeded = getParseSucceeded(result);
        return ParserFailed{"Impossible state in runParser", succeeded.from};
      }
  }

  if (isLeft(visitor.result))
  {
    ParserFailed failed = getParseFailed(visitor.result);
    runtime.pushMessage("runParser: parsing failed at position " + std::to_string(failed.at) + ": " + failed.message);
    return failed;
  }
  else
  {
    ParserSucceeded<Ret> succeeded = getParseSucceeded(visitor.result);
    succeeded.from = startFrom;   // Update the from position to be the startFrom passed to runParser, since the visitor might have updated it during the parsing process. This way we have a more accurate from position in the final result.
    runtime.pushMessage("runParser: parsing succeeded. From: " + std::to_string(succeeded.from) + ", To: " + std::to_string(succeeded.to) + ".");
    return succeeded;
  }
}


template <typename Ret>
struct InterpretingADTVisitor
{
    ParserRuntime& _runtime;
    ParserResult<Ret> result;
    Pos _startFrom;

    InterpretingADTVisitor(ParserRuntime& runtime, Pos startFrom)
        : _runtime(runtime), _startFrom(startFrom)
    {
    }

    void operator()(const ParseSymbolCond<Parser<Ret>>& method)
    {
        ParserResult<Char> r = ParserFailed{"Internal error: null parser in ParseSymbolCond.", _startFrom};  // Default value in case of an error, will be overwritten if the parser is not null.

        _runtime.pushMessage("ParseSymbolCond: starting parseSingle. Current position: " + std::to_string(_startFrom) + ".");
        try
        {
            r = parseSingle<Char>(_runtime, _startFrom, method.validator, method.name);
        }
        catch (const std::exception& e)
        {
            _runtime.pushMessage(std::string("Exception in parseSingle: ") + e.what());
            result = ParserFailed{"Internal error: exception in parseSingle: " + std::string(e.what()), _startFrom};
            return;
        }

        if (isLeft(r))
        {
          ParserFailed failed = getParseFailed(r);
          result = ParserFailed{failed.message, failed.at};
        }
        else
        {
            ParserSucceeded<Char> succeeded = getParseSucceeded(r);
            Parser<Ret> rNext = method.next(succeeded.parsed);
            result = runParser<Ret>(_runtime, rNext, succeeded.to);
        }
    }

    void operator()(const ParseLit<Parser<Ret>> &method)
    {
      ParserResult<std::string> r = ParserFailed{"Internal error: null parser in ParseLit.", _startFrom};  // Default value in case of an error, will be overwritten if the parser is not null.

      _runtime.pushMessage("ParseLit: starting parseLit. Current position: " + std::to_string(_startFrom) + ".");
      try
      {
          r = parseLit<std::string>(_runtime, _startFrom, method.s);
      }
      catch (const std::exception& e)
      {
          _runtime.pushMessage(std::string("Exception in parseLit: ") + e.what());
          result = ParserFailed{"Internal error: exception in parseLit: " + std::string(e.what()), _startFrom};
          return;
      }

      if (isLeft(r))
      {
        ParserFailed failed = getParseFailed(r);
        result = ParserFailed{failed.message, failed.at};
      }
      else
      {
        ParserSucceeded<std::string> succeeded = getParseSucceeded(r);
        Parser<Ret> rNext = method.next(succeeded.parsed);
        result = runParser<Ret>(_runtime, rNext, succeeded.to);
      }
    }

    void operator()(const ParseMany<Parser<Ret>>& method)
    {
      if (method.rawParser == nullptr)
      {
        _runtime.pushMessage("ParseMany: rawParser is null.");
        result = ParserFailed { "Internal error: null parser in ParseMany.", _startFrom };
        return;
      }

      // Create a temporary runtime to run the raw parser, so that we don't modify the original runtime's state and messages during the loop.
      // N.B. For now, the source string is copied. It is highly suboptimal.

      std::list<Any> acc;
      Pos currentPos = _startFrom;
      int iteration = 0;

      _runtime.pushMessage("ParseMany: starting first runParser. Current position: " + std::to_string(currentPos) + ".");
      ParserResult<Any> r = runParser<Any>(_runtime, *method.rawParser, currentPos);
      _runtime.pushMessage("ParseMany: entering loop.");

      while (isRight(r))
      {
        ParserSucceeded<Any> succeeded = getParseSucceeded(r);
        acc.push_back(succeeded.parsed);    // Result will be converted when fmapped

        currentPos = succeeded.to;
        _runtime.pushMessage("ParseMany: current position after parsing: " + std::to_string(currentPos) + ". Parsed value added to accumulator, size now: " + std::to_string(acc.size()) + ".");
        iteration++;
        r = runParser<Any>(_runtime, *method.rawParser, currentPos);

        if (iteration > _runtime.getManyCombinatorThreshold())   // Safety check to prevent infinite loops, in case of a bug in the raw parser or something like that. In a real implementation we might want to handle this differently, maybe by throwing an exception or something like that.
        {
          _runtime.pushMessage("ParseMany: safety check triggered, breaking loop after " + std::to_string(iteration) + " iterations.");
          break;
        }
      }
      _runtime.pushMessage("ParseMany: loop ended after " + std::to_string(iteration) + " iterations.");

      result = runParser<Ret>(_runtime, method.next(acc), currentPos);
      _runtime.pushMessage("ParseMany: finished.");
    }

    void operator()(const TryOrErrorParser<Parser<Ret>>& method)
    {
      if (method.rawParser == nullptr)
      {
        _runtime.pushMessage("TryOrError: rawParser is null.");
        result = ParserFailed{"Internal error: null parser in TryOrError.", _startFrom};
        return;
      }

      // Create a temporary runtime to run the raw parser, so that we don't modify the original runtime's state and messages during the loop.
      // N.B. For now, the source string is copied. It is highly suboptimal.
      ParserRuntime tempRuntime = _runtime;

      tempRuntime.pushMessage("TryOrError: starting runParser. Current position: " + std::to_string(_startFrom) + ".");
      ParserResult<Any> r = runParser<Any>(tempRuntime, *method.rawParser, _startFrom);

      auto messages = tempRuntime.getMessages(); // Get messages from the temporary runtime and push them to the original runtime, so that we have the messages from all iterations of the loop.
      for (const auto &msg : messages)
      {
        _runtime.pushMessage(msg);
      }

      if (isRight(r))
      {
        ParserSucceeded<Any> succeeded = getParseSucceeded(r);
        _runtime.pushMessage("TryOrError: raw parser succeeded. From: " + std::to_string(succeeded.from) + ", To: " + std::to_string(succeeded.to) + ".");
        result = runParser<Ret>(_runtime, method.next(succeeded), succeeded.to);
      }
      else
      {
        ParserFailed failed = getParseFailed(r);
        _runtime.pushMessage("TryOrError: raw parser failed at " + std::to_string(failed.at)
          + " Resetting position to " + std::to_string(_startFrom) + " for the next parser.");
        failed.at = _startFrom;   // Reset the position to the original position, since try should not consume input on failure.
        result = runParser<Ret>(_runtime, method.next(failed), _startFrom);
      }
    }

    void operator()(const AltParser<Parser<Ret>>& method)
    {
      if (method.p == nullptr || method.q == nullptr)
      {
        _runtime.pushMessage("AltParser: one of the parsers is null.");
        result = ParserFailed{"Internal error: null parser in AltParser.", _startFrom};
        return;
      }

      ParserRuntime tempRuntime = _runtime;   // Create a temporary runtime to run the first parser, so that we don't modify the original runtime's state and messages during the first attempt.

      _runtime.pushMessage("AltParser: starting first parser. Current position: " + std::to_string(_startFrom) + ".");
      ParserResult<Any> r = runParser<Any>(tempRuntime, *method.p, _startFrom);

      auto messages = tempRuntime.getMessages(); // Get messages from the temporary runtime and push them to the original runtime, so that we have the messages from all iterations of the loop.
      for (const auto &msg : messages)
      {
          _runtime.pushMessage(msg);
      }

      if (isRight(r))
      {
        ParserSucceeded<Any> succeeded = getParseSucceeded(r);
        _runtime.pushMessage("AltParser: first parser succeeded. From: " + std::to_string(succeeded.from) + ", To: " + std::to_string(succeeded.to) + ".");
        result = runParser<Ret>(_runtime, method.next(succeeded.parsed), succeeded.to);
        return;
      }

      auto failed = getParseFailed(r);

      if (failed.at != _startFrom)
      {
        _runtime.pushMessage("AltParser: first parser failed at " + std::to_string(failed.at) + ", which is different from the starting position: " + std::to_string(_startFrom) + " (consumed input). Not trying second parser.");
        return;
      }

      _runtime.pushMessage("AltParser: first parser failed without consumption. Starting second parser. Current position: " + std::to_string(_startFrom) + ".");
      tempRuntime.clearMessages();

      ParserResult<Any> r2 = runParser<Any>(tempRuntime, *method.q, _startFrom);

      auto messages2 = tempRuntime.getMessages(); // Get messages from the temporary runtime and push them to the original runtime, so that we have the messages from all iterations of the loop.
      for (const auto &msg : messages2)
      {
          _runtime.pushMessage(msg);
      }

      if (isRight(r2))
      {
        ParserSucceeded<Any> succeeded2 = getParseSucceeded(r2);
        _runtime.pushMessage("AltParser: second parser succeeded. From: " + std::to_string(_startFrom) + ", To: " + std::to_string(succeeded2.to) + ".");
        result = runParser<Ret>(_runtime, method.next(succeeded2.parsed), succeeded2.to);
      }
      else
      {
        ParserFailed failed2 = getParseFailed(r2);
        _runtime.pushMessage("AltParser: second parser failed at " + std::to_string(failed2.at) + ".");
        result = ParserFailed{std::string("Both parsers failed. First parser error message: ") + failed.message + " Second parser error message: " + failed2.message, failed2.at};
      }
    }

    void operator()(const GetSt<Parser<Ret>> &method)
    {
      auto rNext = method.next(_runtime.getState());
      result = runParser<Ret>(_runtime, rNext, _startFrom);
    }

    void operator()(const PutSt<Parser<Ret>> &method)
    {
      _runtime.putState(method.st);
      auto rNext = method.next(unit);
      result = runParser<Ret>(_runtime, rNext, _startFrom);
    }
};

template <typename Ret>
struct InterpretingVisitor
{
    ParserRuntime& _runtime;
    ParserResult<Ret> result;
    Pos _startFrom;

    InterpretingVisitor(ParserRuntime& runtime, Pos startFrom)
        : _runtime(runtime)
        , result(ParserFailed{"Internal error: null parser in InterpretingVisitor.", startFrom})  // Default value in case of an error, will be overwritten if the parser is not null.
        , _startFrom(startFrom)
    {
    }

    void operator()(const PureF<Ret>& p)
    {
      result = ParserSucceeded<Ret>{p.ret, _startFrom, _startFrom};
    }

    void operator()(const FreeF<Ret>& f)
    {
      InterpretingADTVisitor<Ret> visitor(_runtime, _startFrom);
      std::visit(visitor, f.psf.psf);
      result = visitor.result;
    }
};

} // namespace core
} // namespace ps

#endif // PS_CORE_INTERPRETER_H
