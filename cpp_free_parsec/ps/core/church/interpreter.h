#ifndef PS_CORE_CHURCH_INTERPRETER_H
#define PS_CORE_CHURCH_INTERPRETER_H

#include "../types.h"
#include "../methods/adt.h"
#include "adt.h"

#include "../runtime.h"
#include "../internal/raw_parsers.h"


namespace ps
{
namespace core
{
namespace church
{

template <typename Ret>
ParserResult<Ret> runParser(
        ParserRuntime& runtime,
        const Parser<Ret>& next,
        Pos startFrom,
        const std::string& indent = "")
{
  // Visitor holds state used by the p and r callbacks
  struct InterpretingVisitor
  {
    ParserRuntime& _runtime;
    ParserResult<Ret> result;
    Pos _startFrom;
    Pos _currentTo;
    std::string _indent;
    std::string _parserDebugInfo;
    std::string _successMsg;

    InterpretingVisitor(ParserRuntime& runtime, Pos startFrom, const std::string& indent,
                        std::string parserDebugInfo)
        : _runtime(runtime)
        , result(ParserFailed{"", startFrom})
        , _startFrom(startFrom)
        , _currentTo(startFrom)
        , _indent(indent)
        , _parserDebugInfo(parserDebugInfo)
    {
    }
  };

  InterpretingVisitor visitor(runtime, startFrom, indent + "  ", next.debugInfo);

  auto x = std::to_string(startFrom);
  auto paddedTo4Symb = std::string(4 - x.length(), ' ') + x;

  // p: called by the church-encoded parser when a pure value is produced.
  std::function<Any(Ret)> p = [&](const Ret &a) -> Any
  {
    ParserSucceeded<Ret> succeeded;
    succeeded.parsed = a;
    succeeded.from = visitor._startFrom;
    succeeded.to = visitor._currentTo;
    visitor.result = succeeded;
    return Any{};////////////////////////////////////////////MISTAKE!
  };

  // r: called by the church-encoded parser when it exposes a ParserMethods to be executed.
  std::function<Any(ParserMethods<Any, Parser>)> r = [&](const ParserMethods<Any, Parser> &methodAny) -> Any
  {
    // Visitor for the methods variant
    struct MethodsVisitor
    {
      InterpretingVisitor &outer;
      const ParserMethods<Any, Parser> &methodAnyRef;

      MethodsVisitor(InterpretingVisitor &o, const ParserMethods<Any, Parser> &m)
        : outer(o), methodAnyRef(m) {}

      void operator()(const ParseSymbolCond<Any, Parser>& method)
      {
        ParserResult<Char> r = ParserFailed{"Fail", outer._startFrom};

        auto x = std::to_string(outer._startFrom);
        auto paddedTo4Symb = std::string(4 - x.length(), ' ') + x;

        try
        {
            r = parseSingle<Char>(outer._runtime, outer._currentTo, method.validator);
        }
        catch (const std::exception& e)
        {
            outer.result = ParserFailed{"Exception in parseSingle: " + std::string(e.what()), outer._startFrom};
            return;
        }

        if (isLeft(r))
        {
          ParserFailed failed = getParseFailed(r);
          outer.result = ParserFailed{failed.message, failed.at};
        }
        else
        {
            ParserSucceeded<Char> succeeded = getParseSucceeded(r);
            outer._currentTo = succeeded.to;
            outer._successMsg = std::string("Parsed char: '") + succeeded.parsed + "'";
            outer._runtime.pushMessage("[" + paddedTo4Symb + "] " +
                outer._indent.substr(0, outer._indent.size() - 2)
                + "<" + outer._parserDebugInfo + "> success " + outer._successMsg);

            // Call the continuation produced by fmapMethods (it will call p internally).
            try
            {
                method.next(succeeded.parsed);
            }
            catch (const std::exception &e)
            {
                outer.result = ParserFailed{std::string("Exception in continuation: ") + e.what(), outer._startFrom};
            }
        }
      }

      void operator()(const ParseLit<Any, Parser>& method)
      {
        ParserResult<std::string> r = ParserFailed{"Fail", outer._startFrom};

        auto x = std::to_string(outer._startFrom);
        auto paddedTo4Symb = std::string(4 - x.length(), ' ') + x;

        try
        {
            r = parseLit<std::string>(outer._runtime, outer._currentTo, method.s);
        }
        catch (const std::exception& e)
        {
            outer.result = ParserFailed{"Exception in parseLit: " + std::string(e.what()), outer._startFrom};
            return;
        }

        if (isLeft(r))
        {
          ParserFailed failed = getParseFailed(r);
          outer.result = ParserFailed{failed.message, failed.at};
        }
        else
        {
          ParserSucceeded<std::string> succeeded = getParseSucceeded(r);
          outer._currentTo = succeeded.to;
          outer._successMsg = std::string("Parsed lit: '") + succeeded.parsed + "'";
          outer._runtime.pushMessage("[" + paddedTo4Symb + "] " +
                              outer._indent.substr(0, outer._indent.size() - 2) + "<" + outer._parserDebugInfo + "> success " + outer._successMsg);

          try
          {
              method.next(succeeded.parsed);
          }
          catch (const std::exception &e)
          {
              outer.result = ParserFailed{std::string("Exception in continuation: ") + e.what(), outer._startFrom};
          }
        }
      }

      void operator()(const ParseMany<Any, Parser>& method)
      {
        if (method.rawParser == nullptr)
        {
          outer.result = ParserFailed { "Null parser in ParseMany", outer._startFrom };
          return;
        }

        std::list<Any> acc;
        Pos currentPos = outer._currentTo;
        int iteration = 0;

        ParserRuntime tempRuntime = outer._runtime;
        tempRuntime.clearMessages();
        ParserResult<Any> rr = runParser<Any>(tempRuntime, *method.rawParser, currentPos, outer._indent);

        auto messages = tempRuntime.getMessages();
        for (const auto &msg : messages)
        {
            outer._runtime.pushMessage(msg);
        }

        while (isRight(rr))
        {
          ParserSucceeded<Any> succeeded = getParseSucceeded(rr);
          acc.push_back(succeeded.parsed);

          currentPos = succeeded.to;
          iteration++;

          tempRuntime.clearMessages();
          rr = runParser<Any>(tempRuntime, *method.rawParser, currentPos, outer._indent);

          messages = tempRuntime.getMessages();
          for (const auto &msg : messages)
          {
              outer._runtime.pushMessage(msg);
          }

          if (iteration > outer._runtime.getManyCombinatorThreshold())
          {
            outer._runtime.pushMessage("ParseMany: safety check triggered, breaking loop after " + std::to_string(iteration) + " iterations.");
            break;
          }
        }

        outer._currentTo = currentPos;

        try
        {
            method.next(acc);
        }
        catch (const std::exception &e)
        {
            outer.result = ParserFailed{std::string("Exception in continuation: ") + e.what(), outer._startFrom};
        }
      }

      void operator()(const TryOrErrorParser<Any, Parser>& method)
      {
        if (method.rawParser == nullptr)
        {
          outer.result = ParserFailed{"Null parser in TryOrError.", outer._startFrom};
          return;
        }

        ParserRuntime tempRuntime = outer._runtime;
        tempRuntime.clearMessages();
        ParserResult<Any> r = runParser<Any>(tempRuntime, *method.rawParser, outer._startFrom, outer._indent);

        auto messages = tempRuntime.getMessages();
        for (const auto &msg : messages)
        {
          outer._runtime.pushMessage(msg);
        }

        if (isRight(r))
        {
          ParserSucceeded<Any> succeeded = getParseSucceeded(r);
          outer._currentTo = succeeded.to;
          try
          {
              method.next(r);
          }
          catch (const std::exception &e)
          {
              outer.result = ParserFailed{std::string("Exception in continuation: ") + e.what(), outer._startFrom};
          }
        }
        else
        {
          ParserFailed failed = getParseFailed(r);
          failed.at = outer._startFrom;
          outer._currentTo = outer._startFrom;
          try
          {
              method.next(r);
          }
          catch (const std::exception &e)
          {
              outer.result = ParserFailed{std::string("Exception in continuation: ") + e.what(), outer._startFrom};
          }
        }
      }

      void operator()(const AltParser<Any, Parser>& method)
      {
        if (method.p == nullptr)
        {
          outer.result = ParserFailed{"Null parser in AltParser (left)", outer._startFrom};
          return;
        }
        if (method.q == nullptr)
        {
          outer.result = ParserFailed{"Null parser in AltParser (right)", outer._startFrom};
          return;
        }

        ParserRuntime tempRuntime = outer._runtime;
        tempRuntime.clearMessages();
        ParserResult<Any> r = runParser<Any>(tempRuntime, *method.p, outer._startFrom, outer._indent);

        auto messages = tempRuntime.getMessages();
        for (const auto &msg : messages)
        {
            outer._runtime.pushMessage(msg);
        }

        if (isRight(r))
        {
          ParserSucceeded<Any> succeeded = getParseSucceeded(r);
          outer._currentTo = succeeded.to;
          try
          {
              method.next(succeeded.parsed);
          }
          catch (const std::exception &e)
          {
              outer.result = ParserFailed{std::string("Exception in continuation: ") + e.what(), outer._startFrom};
          }
          return;
        }

        auto failed = getParseFailed(r);

        if (failed.at != outer._startFrom)
        {
          outer.result = ParserFailed{"No alt: first consumed", failed.at};
          return;
        }

        tempRuntime.clearMessages();
        ParserResult<Any> r2 = runParser<Any>(tempRuntime, *method.q, outer._startFrom, outer._indent);

        auto messages2 = tempRuntime.getMessages();
        for (const auto &msg : messages2)
        {
            outer._runtime.pushMessage(msg);
        }

        if (isRight(r2))
        {
          ParserSucceeded<Any> succeeded2 = getParseSucceeded(r2);
          outer._currentTo = succeeded2.to;
          try
          {
              method.next(succeeded2.parsed);
          }
          catch (const std::exception &e)
          {
              outer.result = ParserFailed{std::string("Exception in continuation: ") + e.what(), outer._startFrom};
          }
        }
        else
        {
          ParserFailed failed2 = getParseFailed(r2);
          outer.result = ParserFailed{"No alt: both failed", failed2.at};
        }
      }

      void operator()(const LazyParser<Any, Parser>& method)
      {
        Parser<Any> actualParser = method.parserFactory();
        ParserResult<Any> r = runParser<Any>(outer._runtime, actualParser, outer._startFrom, outer._indent);

        if (isLeft(r))
        {
          ParserFailed failed = getParseFailed(r);
          outer.result = ParserFailed{failed.message, failed.at};
        }
        else
        {
          ParserSucceeded<Any> succeeded = getParseSucceeded(r);
          outer._currentTo = succeeded.to;
          try
          {
              method.next(succeeded.parsed);
          }
          catch (const std::exception &e)
          {
              outer.result = ParserFailed{std::string("Exception in continuation: ") + e.what(), outer._startFrom};
          }
        }
      }

      void operator()(const GetSt<Any, Parser>& method)
      {
        try
        {
            method.next(outer._runtime.getState());
        }
        catch (const std::exception &e)
        {
            outer.result = ParserFailed{std::string("Exception in continuation: ") + e.what(), outer._startFrom};
        }
      }

      void operator()(const PutSt<Any, Parser>& method)
      {
        outer._runtime.putState(method.st);
        try
        {
            method.next(unit);
        }
        catch (const std::exception &e)
        {
            outer.result = ParserFailed{std::string("Exception in continuation: ") + e.what(), outer._startFrom};
        }
      }
    };

    MethodsVisitor mv(visitor, methodAny);
    std::visit(mv, const_cast<ParserMethods<Any, Parser>&>(methodAny).psf);

    return Any{};
  };

  try
  {
    runtime.pushMessage("[" + paddedTo4Symb + "] " + indent + "<" + next.debugInfo + ">");
    next.runF(p, r);
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





} // namespace church
} // namespace core
} // namespace ps

#endif // PS_CORE_CHURCH_INTERPRETER_H
