#ifndef PS_CORE_FREE_INTERPRETER_H
#define PS_CORE_FREE_INTERPRETER_H

#include "../types.h"
#include "../methods/adt.h"
#include "../methods/functor.h"
#include "adt.h"
#include "bind.h"
#include "functor.h"

#include "../runtime.h"
#include "../internal/raw_parsers.h"


namespace ps
{
namespace core
{
namespace free
{

// Forward declaration
template <typename Ret>
struct InterpretingVisitor;


template <typename Ret>
ParserResult<Ret> runParser(
        ParserRuntime& runtime,
        const Parser<Ret>& next,
        Pos startFrom,
        const std::string& indent = "")
{
  InterpretingVisitor<Ret> visitor(runtime, startFrom, indent + "  ", next.debugInfo);

  auto x = std::to_string(startFrom);
  auto paddedTo4Symb = std::string(4 - x.length(), ' ') + x;

  try
  {
    runtime.pushMessage("[" + paddedTo4Symb + "] " + indent + "<" + next.debugInfo + ">");
    std::visit(visitor, next.psl);
  }
  catch (const std::exception& e)
  {
      auto result = visitor.result;
      if (isLeft(result))
      {
        ParserFailed failed = getParseFailed(result);
        return ParserFailed{"<" + next.debugInfo + "> exception: " + std::string(e.what()) + ". Msg: " + failed.message, failed.at};
      }
      else
      {
        // Should not happen
        ParserSucceeded<Ret> succeeded = getParseSucceeded(result);
        return ParserFailed{"<" + next.debugInfo + "> impossible " + std::string(e.what()), succeeded.from};
      }
  }

  if (isLeft(visitor.result))
  {
    ParserFailed failed = getParseFailed(visitor.result);
    runtime.pushMessage("[" + paddedTo4Symb + "] " + indent + "<" + next.debugInfo + "> failed: " + failed.message);
    return failed;
  }
  else
  {
    ParserSucceeded<Ret> succeeded = getParseSucceeded(visitor.result);
    succeeded.from = startFrom;
    runtime.pushMessage("[" + paddedTo4Symb + "] " + indent + "<" + next.debugInfo + "> success " + visitor._successMsg);
    return succeeded;
  }
}


template <typename Ret>
struct InterpretingADTVisitor
{
    ParserRuntime& _runtime;
    ParserResult<Ret> result;
    Pos _startFrom;
    std::string _indent;
    std::string _parserDebugInfo;
    std::string _successMsg;

    InterpretingADTVisitor(ParserRuntime& runtime, Pos startFrom, const std::string& indent,
      std::string parserDebugInfo)
        : _runtime(runtime)
        , _startFrom(startFrom)
        , _indent(indent)
        , _parserDebugInfo(parserDebugInfo)
    {
    }

    void operator()(const ParseSymbolCond<Parser<Ret>, Parser>& method)
    {
        ParserResult<Char> r = ParserFailed{"Fail", _startFrom};

        auto x = std::to_string(_startFrom);
        auto paddedTo4Symb = std::string(4 - x.length(), ' ') + x;

        try
        {
            r = parseSingle<Char>(_runtime, _startFrom, method.validator);
        }
        catch (const std::exception& e)
        {
            result = ParserFailed{"Exception in parseSingle: " + std::string(e.what()), _startFrom};
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
            _successMsg = std::string("Parsed char: '") + succeeded.parsed + "'";
            _runtime.pushMessage("[" + paddedTo4Symb + "] " +
                _indent.substr(0, _indent.size() - 2)
                + "<" + _parserDebugInfo + "> success " + _successMsg);
            result = runParser<Ret>(_runtime, rNext, succeeded.to, _indent);
        }
    }

    void operator()(const ParseLit<Parser<Ret>, Parser>& method)
    {
      ParserResult<std::string> r = ParserFailed{"Fail", _startFrom};

      auto x = std::to_string(_startFrom);
      auto paddedTo4Symb = std::string(4 - x.length(), ' ') + x;

      try
      {
          r = parseLit<std::string>(_runtime, _startFrom, method.s);
      }
      catch (const std::exception& e)
      {
          result = ParserFailed{"Exception in parseLit: " + std::string(e.what()), _startFrom};
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
        _successMsg = std::string("Parsed lit: '") + succeeded.parsed + "'";
        _runtime.pushMessage("[" + paddedTo4Symb + "] " +
                            _indent.substr(0, _indent.size() - 2) + "<" + _parserDebugInfo + "> success " + _successMsg);
        result = runParser<Ret>(_runtime, rNext, succeeded.to, _indent);
      }
    }

    void operator()(const ParseMany<Parser<Ret>, Parser>& method)
    {
      if (method.rawParser == nullptr)
      {
        result = ParserFailed { "Null parser in ParseMany", _startFrom };
        return;
      }

      // Create a temporary runtime to run the raw parser, so that we don't modify the original runtime's state and messages during the loop.
      // N.B. For now, the source string is copied. It is highly suboptimal.

      std::list<Any> acc;
      Pos currentPos = _startFrom;
      int iteration = 0;

      ParserRuntime tempRuntime = _runtime;
      tempRuntime.clearMessages();
      ParserResult<Any> r = runParser<Any>(tempRuntime, *method.rawParser, currentPos, _indent);

      auto messages = tempRuntime.getMessages();
      for (const auto &msg : messages)
      {
          _runtime.pushMessage(msg);
      }

      while (isRight(r))
      {
        ParserSucceeded<Any> succeeded = getParseSucceeded(r);
        acc.push_back(succeeded.parsed);

        currentPos = succeeded.to;
        iteration++;

        tempRuntime.clearMessages();
        r = runParser<Any>(tempRuntime, *method.rawParser, currentPos, _indent);

        messages = tempRuntime.getMessages();
        for (const auto &msg : messages)
        {
            _runtime.pushMessage(msg);
        }

        if (iteration > _runtime.getManyCombinatorThreshold())   // Safety check to prevent infinite loops, in case of a bug in the raw parser or something like that. In a real implementation we might want to handle this differently, maybe by throwing an exception or something like that.
        {
          _runtime.pushMessage("ParseMany: safety check triggered, breaking loop after " + std::to_string(iteration) + " iterations.");
          break;
        }
      }

      result = runParser<Ret>(_runtime, method.next(acc), currentPos, _indent);
    }

    void operator()(const TryOrErrorParser<Parser<Ret>, Parser> &method)
    {
      if (method.rawParser == nullptr)
      {
        result = ParserFailed{"Null parser in TryOrError.", _startFrom};
        return;
      }

      // Create a temporary runtime to run the raw parser, so that we don't modify the original runtime's state and messages during the loop.
      // N.B. For now, the source string is copied. It is highly suboptimal.
      ParserRuntime tempRuntime = _runtime;
      tempRuntime.clearMessages();
      ParserResult<Any> r = runParser<Any>(tempRuntime, *method.rawParser, _startFrom, _indent);

      auto messages = tempRuntime.getMessages();
      for (const auto &msg : messages)
      {
        _runtime.pushMessage(msg);
      }

      if (isRight(r))
      {
        ParserSucceeded<Any> succeeded = getParseSucceeded(r);
        result = runParser<Ret>(_runtime, method.next(succeeded), succeeded.to, _indent);
      }
      else
      {
        ParserFailed failed = getParseFailed(r);
        failed.at = _startFrom;   // Reset the position to the original position, since try should not consume input on failure.
        result = runParser<Ret>(_runtime, method.next(failed), _startFrom, _indent);
      }
    }

    void operator()(const AltParser<Parser<Ret>, Parser> &method)
    {
      if (method.p == nullptr)
      {
        result = ParserFailed{"Null parser in AltParser (left)", _startFrom};
        return;
      }
      if (method.q == nullptr)
      {
        result = ParserFailed{"Null parser in AltParser (right)", _startFrom};
        return;
      }

      ParserRuntime tempRuntime = _runtime;

      tempRuntime.clearMessages();
      ParserResult<Any> r = runParser<Any>(tempRuntime, *method.p, _startFrom, _indent);

      auto messages = tempRuntime.getMessages();
      for (const auto &msg : messages)
      {
          _runtime.pushMessage(msg);
      }

      if (isRight(r))
      {
        ParserSucceeded<Any> succeeded = getParseSucceeded(r);
        result = runParser<Ret>(_runtime, method.next(succeeded.parsed), succeeded.to, _indent);
        return;
      }

      auto failed = getParseFailed(r);

      if (failed.at != _startFrom)
      {
        result = ParserFailed{"No alt: first consumed", failed.at};
        return;
      }

      tempRuntime.clearMessages();
      ParserResult<Any> r2 = runParser<Any>(tempRuntime, *method.q, _startFrom, _indent);

      auto messages2 = tempRuntime.getMessages();
      for (const auto &msg : messages2)
      {
          _runtime.pushMessage(msg);
      }

      if (isRight(r2))
      {
        ParserSucceeded<Any> succeeded2 = getParseSucceeded(r2);
        result = runParser<Ret>(_runtime, method.next(succeeded2.parsed), succeeded2.to, _indent);
      }
      else
      {
        ParserFailed failed2 = getParseFailed(r2);
        result = ParserFailed{"No alt: both failed", failed2.at};
      }
    }

    void operator()(const LazyParser<Parser<Ret>, Parser> &method)
    {
      Parser<Any> actualParser = method.parserFactory();
      ParserRuntime tempRuntime = _runtime;
      tempRuntime.clearMessages();
      ParserResult<Any> r = runParser<Any>(tempRuntime, actualParser, _startFrom, _indent);

      auto messages = tempRuntime.getMessages();
      for (const auto &msg : messages)
      {
        _runtime.pushMessage(msg);
      }

      if (isLeft(r))
      {
        ParserFailed failed = getParseFailed(r);
        result = ParserFailed{failed.message, failed.at};
      }
      else
      {
        ParserSucceeded<Any> succeeded = getParseSucceeded(r);
        result = runParser<Ret>(_runtime, method.next(succeeded.parsed), succeeded.to, _indent);
      }
    }

    void operator()(const GetSt<Parser<Ret>, Parser> &method)
    {
      auto rNext = method.next(_runtime.getState());
      result = runParser<Ret>(_runtime, rNext, _startFrom, _indent);
    }

    void operator()(const PutSt<Parser<Ret>, Parser> &method)
    {
      _runtime.putState(method.st);
      auto rNext = method.next(unit);
      result = runParser<Ret>(_runtime, rNext, _startFrom, _indent);
    }
};

template <typename Ret>
struct InterpretingVisitor
{
    ParserRuntime& _runtime;
    ParserResult<Ret> result;
    Pos _startFrom;
    std::string _indent;
    std::string _parserDebugInfo;
    std::string _successMsg;

    InterpretingVisitor(ParserRuntime& runtime, Pos startFrom, const std::string& indent,
      std::string parserDebugInfo)
        : _runtime(runtime)
        , result(ParserFailed{"", startFrom})
        , _startFrom(startFrom)
        , _indent(indent)
        , _parserDebugInfo(parserDebugInfo)
    {
    }

    void operator()(const PureF<Ret>& p)
    {
      result = ParserSucceeded<Ret>{p.ret, _startFrom, _startFrom};
    }

    void operator()(const FreeF<Ret>& f)
    {
      InterpretingADTVisitor<Ret> visitor(_runtime, _startFrom, _indent, _parserDebugInfo);
      std::visit(visitor, f.psf.psf);
      result = visitor.result;
      _successMsg = visitor._successMsg;
    }
};


} // namespace free
} // namespace core
} // namespace ps

#endif // PS_CORE_FREE_INTERPRETER_H
