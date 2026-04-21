#ifndef PS_CORE_FREE_ADT_H
#define PS_CORE_FREE_ADT_H

#include "../types.h"
#include "../methods/adt.h"

namespace ps
{
namespace core
{
namespace free
{

// Forward declaration
template <typename A>
struct Parser;

// Free language

// Free methods

template <typename Ret>
struct PureF
{
  using ResultType = Ret;
  Ret ret;
};

template <typename Ret>
struct FreeF
{
  using ResultType = Ret;
  ParserMethods<Parser<Ret>, Parser> psf;
};

template <typename A>
struct Parser
{
  using ResultType = A;
  std::variant<PureF<A>, FreeF<A>> psl;
  std::string debugInfo;

  Parser<A> operator+( const std::string& info ) const
  {
    Parser<A> newParser = *this;
    newParser.debugInfo = info;
    return newParser;
  }
};


// TODO: debugInfo is blank by default
template <typename A>
Parser<A> makePure(const A& a, const std::string& debugInfo)
{
  return {PureF<A>{a}, debugInfo};
}

// TODO: debugInfo is blank by default
template <typename A,
          typename Method>
Parser<A> makeFree(const Method& method, const std::string& debugInfo)
{
  return {FreeF<A>{ParserMethods<Parser<A>, Parser>{method}}, debugInfo};
}


} // namespace free
} // namespace core
} // namespace ps

#endif // PS_CORE_FREE_ADT_H
