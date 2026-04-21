#ifndef PS_CORE_ENGINE_H
#define PS_CORE_ENGINE_H

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

namespace ps
{
namespace core
{


#ifdef FREE

template <typename A>
using Parser = ps::core::free::Parser<A>;


// TODO: debugInfo is blank by default
template <typename A,
          typename Method>
Parser<A> wrap(const Method& method, const std::string& debugInfo)
{
  return ps::core::free::makeFree<A, Method>
    (method, debugInfo);
}

// TODO: debugInfo is blank by default
template <typename A>
Parser<A> pure(const A& a, const std::string& debugInfo)
{
  return ps::core::free::makePure<A>(a, debugInfo);
}


template <typename A, typename B>
Parser<B> fmap(
    const std::function<B(A)> &f,
    const Parser<A> &psl)
{
  return ps::core::free::runFMap<A,B>(f, psl);
}


template <typename A, typename B>
Parser<B> bind(const Parser<A> &ma,
               const std::function<Parser<B>(A)> &f)
{
  return ps::core::free::runBind<A, B>(ma, f);
}


#endif // FREE




} // namespace core
} // namespace ps

#endif // PS_CORE_ENGINE_H
