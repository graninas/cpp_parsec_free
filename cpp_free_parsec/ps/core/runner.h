#ifndef PS_CORE_RUNNER_H
#define PS_CORE_RUNNER_H

#include "engine.h"

#ifdef FREE

#include "free/interpreter.h"

#endif // FREE

#ifdef CHURCH

#include "church/interpreter.h"

#endif // CHURCH

#include "runtime.h"

namespace ps
{
namespace core
{


#ifdef FREE

template <typename A>
ParserResult<A> parse(
    const Parser<A> &pst,
    const std::string &s,
    Pos from = 0)
{
  ParserRuntime runtime(s, State{});
  return ps::core::free::runParser(runtime, pst, from);
}

template <typename A>
ParserResult<A> parseWithRuntime(
    ParserRuntime &runtime,
    const Parser<A> &pst,
    Pos from = 0)
{
  return ps::core::free::runParser(runtime, pst, from);
}

#endif // FREE

#ifdef CHURCH

template <typename A>
ParserResult<A> parse(
    const Parser<A> &pst,
    const std::string &s,
    Pos from = 0)
{
  ParserRuntime runtime(s, State{});
  return ps::core::church::runParser(runtime, pst, from);
}

template <typename A>
ParserResult<A> parseWithRuntime(
    ParserRuntime &runtime,
    const Parser<A> &pst,
    Pos from = 0)
{
  ps::core::church::State<A> state(runtime, from, "", pst.debugInfo);
  return ps::core::church::runParser(state, pst);
}

#endif // CHURCH

} // namespace core
} // namespace ps

#endif // PS_CORE_RUNNER_H
