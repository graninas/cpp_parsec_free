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

// Visitor holds state used by the pureFunc and runMethods callbacks
template <typename Ret>
struct State
{
  ParserRuntime& _runtime;
  ParserResult<Ret> result;
  Pos _parsePosition;
  std::string _indent;
  std::string _parserDebugInfo;
  std::string _successMsg = "";

  int _depth = 20;

  State(ParserRuntime& runtime,
    Pos parsePos,
    const std::string& indent,
    std::string parserDebugInfo)
      : _runtime(runtime)
      , result(ParserFailed{"", parsePos})
      , _parsePosition(parsePos)
      , _indent(indent)
      , _parserDebugInfo(parserDebugInfo)
  {
  }
};

// Forward
template <typename Ret>
ParserResult<Ret> runParser(State<Ret>& state, const Parser<Ret>& parser);

  template <typename Ret>
  struct MethodsVisitor
  {
    State<Ret> &state;

    MethodsVisitor(State<Ret> &o)
      : state(o)
    {
    }

    void operator()(const ParseSymbolCond<Any, Parser> &method)
    {
      ParserResult<Char> r = ParserFailed{"Fail", state._parsePosition};

      auto x = std::to_string(state._parsePosition);
      auto paddedTo4Symb = std::string(4 - x.length(), ' ') + x;

      try
      {
        r = parseSingle<Char>(state._runtime, state._parsePosition, method.validator);
      }
      catch (const std::exception &e)
      {
        state.result = ParserFailed{"Exception in parseSingle: " + std::string(e.what()), state._parsePosition};
        return;
      }

      if (isLeft(r))
      {
        ParserFailed failed = getParseFailed(r);
        state.result = ParserFailed{failed.message, failed.at};
      }
      else
      {
        ParserSucceeded<Char> succeeded = getParseSucceeded(r);
        state._parsePosition = succeeded.to;
        state._successMsg = std::string("(Char '") + succeeded.parsed + "')";
        state._runtime.pushMessage("[" + paddedTo4Symb + "] " +
                             state._indent + "<" + state._parserDebugInfo + "> "
                             + std::string("PARSED CHAR: '") + succeeded.parsed + "'");
        method.next(succeeded.parsed);
      }
    }

    void operator()(const ParseLit<Any, Parser> &method)
    {
      ParserResult<std::string> r = ParserFailed{"Fail", state._parsePosition};

      auto x = std::to_string(state._parsePosition);
      auto paddedTo4Symb = std::string(4 - x.length(), ' ') + x;

      try
      {
        r = parseLit<std::string>(state._runtime, state._parsePosition, method.s);
      }
      catch (const std::exception &e)
      {
        state.result = ParserFailed{"Exception in parseLit: " + std::string(e.what()), state._parsePosition};
        return;
      }

      if (isLeft(r))
      {
        ParserFailed failed = getParseFailed(r);
        state.result = ParserFailed{failed.message, failed.at};
      }
      else
      {
        ParserSucceeded<std::string> succeeded = getParseSucceeded(r);
        state._parsePosition = succeeded.to;
        state._successMsg = std::string("(Lit: '") + succeeded.parsed + "')";
        state._runtime.pushMessage("[" + paddedTo4Symb + "] " +
                             state._indent + "<" + state._parserDebugInfo + "> "
                             + std::string("PARSED LIT: '") + succeeded.parsed + "'");
        method.next(succeeded.parsed);
      }
    }

    void operator()(const ParseMany<Any, Parser> &method)
    {
      if (method.rawParser == nullptr)
      {
        state.result = ParserFailed{"Null parser in ParseMany", state._parsePosition};
        return;
      }

      std::list<Any> acc;
      int iteration = 0;

      ParserRuntime tempRuntime = state._runtime.cloneClean();
      State<Any> tempState(tempRuntime, state._parsePosition, state._indent + "  ", method.rawParser->debugInfo);
      ParserResult<Any> rr = runParser<Any>(tempState, *method.rawParser);
      state._runtime.adoptMessages(tempRuntime);

      while (isRight(rr))
      {
        ParserSucceeded<Any> succeeded = getParseSucceeded(rr);
        acc.push_back(succeeded.parsed);

        iteration++;

        tempRuntime.clearMessages();
        tempState._parsePosition = succeeded.to;
        rr = runParser<Any>(tempState, *method.rawParser);

        state._runtime.adoptMessages(tempRuntime);

        if (iteration > state._runtime.getManyCombinatorThreshold())
        {
          state._runtime.pushMessage("ParseMany: safety check triggered, breaking loop after " + std::to_string(iteration) + " iterations.");
          break;
        }
      }

      state._parsePosition = tempState._parsePosition;
      method.next(acc);
    }

    void operator()(const TryOrErrorParser<Any, Parser> &method)
    {
      if (method.rawParser == nullptr)
      {
        state.result = ParserFailed{"Null parser in TryOrError.", state._parsePosition};
        return;
      }

      Pos initialPosition = state._parsePosition;

      ParserRuntime tempRuntime = state._runtime.cloneClean();
      State<Any> tempState(tempRuntime, initialPosition, state._indent + "  ", method.rawParser->debugInfo);
      ParserResult<Any> r = runParser<Any>(tempState, *method.rawParser);
      state._runtime.adoptMessages(tempRuntime);

      if (isRight(r))
      {
        ParserSucceeded<Any> succeeded = getParseSucceeded(r);
        state._parsePosition = succeeded.to;
        method.next(r);
      }
      else
      {
        ParserFailed failed = getParseFailed(r);
        state.result = ParserFailed{failed.message, initialPosition};     // Indicating that we didn't consume
        state._parsePosition = initialPosition;
      }
    }

    void operator()(const AltParser<Any, Parser> &method)
    {
      if (method.p == nullptr)
      {
        state.result = ParserFailed{"Null parser in AltParser (left)", state._parsePosition};
        return;
      }
      if (method.q == nullptr)
      {
        state.result = ParserFailed{"Null parser in AltParser (right)", state._parsePosition};
        return;
      }

      Pos initialPosition = state._parsePosition;

      // Try the left parser
      ParserRuntime tempRuntime = state._runtime.cloneClean();
      State<Any> tempState(tempRuntime, initialPosition, state._indent + "  ", method.p->debugInfo);
      ParserResult<Any> r = runParser<Any>(tempState, *method.p);
      state._runtime.adoptMessages(tempRuntime);

      if (isRight(r))
      {
        ParserSucceeded<Any> succeeded = getParseSucceeded(r);
        state._parsePosition = succeeded.to;
        method.next(succeeded.parsed);
        return;
      }

      auto failed = getParseFailed(r);

      if (failed.at != initialPosition)
      {
        state.result = ParserFailed{"No alt: first consumed", failed.at};
        return;
      }

      // Try the right parser
      tempRuntime.clearMessages();
      tempState._parsePosition = initialPosition;
      ParserResult<Any> r2 = runParser<Any>(tempState, *method.q);
      state._runtime.adoptMessages(tempRuntime);

      if (isRight(r2))
      {
        ParserSucceeded<Any> succeeded2 = getParseSucceeded(r2);
        state._parsePosition = succeeded2.to;
        method.next(succeeded2.parsed);
      }
      else
      {
        ParserFailed failed2 = getParseFailed(r2);
        state.result = ParserFailed{"No alt: both failed", failed2.at};
      }
    }

    void operator()(const LazyParser<Any, Parser> &method)
    {
      Parser<Any> actualParser = method.parserFactory();

      ParserRuntime tempRuntime = state._runtime.cloneClean();
      State<Any> tempState(tempRuntime, state._parsePosition, state._indent + "  ", actualParser.debugInfo);
      ParserResult<Any> r = runParser<Any>(tempState, actualParser);
      state._runtime.adoptMessages(tempRuntime);

      if (isLeft(r))
      {
        ParserFailed failed = getParseFailed(r);
        state.result = ParserFailed{failed.message, failed.at};
      }
      else
      {
        ParserSucceeded<Any> succeeded = getParseSucceeded(r);
        state._parsePosition = succeeded.to;
        method.next(succeeded.parsed);
      }
    }

    void operator()(const GetSt<Any, Parser> &method)
    {
      method.next(state._runtime.getState());
    }

    void operator()(const PutSt<Any, Parser> &method)
    {
      state._runtime.putState(method.st);
      method.next(unit);
    }
  };

template <typename Ret>
ParserResult<Ret> runParser(State<Ret> &state, const Parser<Ret> &parser)
{
  auto initialPosition = state._parsePosition;
  auto x = std::to_string(state._parsePosition);
  auto paddedTo4Symb = std::string(4 - x.length(), ' ') + x;

  // pureFunc: called by the church-encoded parser when a pure value is produced.
  std::function<Any(Ret)> pureFunc = [&](const Ret &a) -> Any
  {
    ParserSucceeded<Ret> succeeded;
    succeeded.parsed = a;
    succeeded.from = state._parsePosition;
    succeeded.to = state._parsePosition;
    state.result = succeeded;
    return Any{};
  };

  // runMethods: called by the church-encoded parser when it exposes a ParserMethods to be executed.
  std::function<Any(ParserMethods<Any, Parser>)> runMethods = [&](const ParserMethods<Any, Parser> &methodAny) -> Any
  {
    MethodsVisitor<Ret> mv(state);
    std::visit(mv, const_cast<ParserMethods<Any, Parser> &>(methodAny).psf);
    return Any{};
  };

  try
  {
    state._runtime.pushMessage("[" + paddedTo4Symb + "] " + state._indent + "<" + parser.debugInfo + ">");
    parser.runF(pureFunc, runMethods);
  }
  catch (const std::exception &e)
  {
    auto result = state.result;
    if (isLeft(result))
    {
      ParserFailed failed = getParseFailed(result);
      return ParserFailed{"<" + parser.debugInfo + "> exception: " + std::string(e.what()) + ". Msg: " + failed.message, failed.at};
    }
    else
    {
      ParserSucceeded<Ret> succeeded = getParseSucceeded(result);
      return ParserFailed{"<" + parser.debugInfo + "> impossible " + std::string(e.what()), succeeded.from};
    }
  }

  if (isLeft(state.result))
  {
    ParserFailed failed = getParseFailed(state.result);
    state._runtime.pushMessage("[" + paddedTo4Symb + "] " + state._indent + "<" + parser.debugInfo + "> failed: " + failed.message);
    return failed;
  }
  else
  {
    ParserSucceeded<Ret> succeeded = getParseSucceeded(state.result);
    succeeded.from = initialPosition;
    state._runtime.pushMessage("[" + paddedTo4Symb + "] " + state._indent + "<" + parser.debugInfo + "> success " + state._successMsg);
    return succeeded;
  }
}


} // namespace church
} // namespace core
} // namespace ps

#endif // PS_CORE_CHURCH_INTERPRETER_H
