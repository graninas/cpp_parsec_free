#ifndef PS_CORE_RUNNER_H
#define PS_CORE_RUNNER_H

#define FREE
// #define CHURCH

#include "types.h"
#include "methods/adt.h"
#include "methods/functor.h"


#ifdef FREE

#include "free/adt.h"
#include "free/functor.h"
#include "free/bind.h"
#include "free/interpreter.h"

#endif // FREE

#ifdef CHURCH

#endif // CHURCH

#include "engine.h"
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




} // namespace core
} // namespace ps

#endif // PS_CORE_RUNNER_H
