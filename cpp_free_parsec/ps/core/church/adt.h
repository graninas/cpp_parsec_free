#ifndef PS_CORE_CHURCH_ADT_H
#define PS_CORE_CHURCH_ADT_H

#include "../types.h"
#include "../methods/adt.h"

namespace ps
{
namespace core
{
namespace church
{

// Forward declaration
template <typename A>
struct Parser;

// Church Free language

template <typename Ret>
using Continuation =
  std::function<
    Any(std::function<Any(Ret)>,
        std::function<Any(ParserMethods<Any, Parser>)>)>;

template <typename A>
struct Parser
{
  using ResultType = A;
  Continuation<A> runF;
  std::string debugInfo;

  Parser<A> &operator+(const std::string &info)
  {
    this->debugInfo = info;
    return *this;
  }
};

// TODO: debugInfo is blank by default
template <typename A>
Parser<A> makePure(const A &a, const std::string &debugInfo)
{
  Parser<A> n;
  n.debugInfo = debugInfo;
  n.runF = [=](const std::function<Any(A)> &p,
               const std::function<Any(ParserMethods<Any, Parser>)> &)
  {
    return p(a);
  };
  return n;
}

// TODO: debugInfo is blank by default
template <typename A, typename Method>
Parser<A> makeChurch(const Method &method, const std::string &debugInfo)
{
  Parser<A> n;

  n.debugInfo = debugInfo;
  n.runF = [=](const std::function<Any(A)> &p,
               const std::function<Any(ParserMethods<Any, Parser>)> &r)
  {
    ParserMethods<A, Parser> methodWrapper{method};

    ParserMethods<Any, Parser> mapped = fmapMethods(p, methodWrapper);
    return r(mapped);
  };

  return n;
}


} // namespace church
} // namespace core
} // namespace ps

#endif // PS_CORE_CHURCH_ADT_H
