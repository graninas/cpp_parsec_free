#ifndef PS_CORE_FREE_FUNCTOR_H
#define PS_CORE_FREE_FUNCTOR_H

#include "../types.h"
#include "../methods/adt.h"
#include "../methods/functor.h"
#include "adt.h"

namespace ps
{
namespace core
{
namespace free
{

// Forward

template <typename A, typename B>
struct FunctorParserVisitor;

// // Free functor

template <typename A, typename B>
Parser<B> runFMap(
    const std::function<B(A)> &f,
    const Parser<A> &psl)
{
  FunctorParserVisitor<A, B> visitor(f);
  std::visit(visitor, psl.psl);
  visitor.result.debugInfo = psl.debugInfo + " (fmap)";
  return visitor.result;
}

template <typename A, typename B>
struct FunctorParserVisitor
{
    std::function<B(A)> fTemplate;
    Parser<B> result;

    FunctorParserVisitor(const std::function<B(A)>& func)
        : fTemplate(func)
    {}

    void operator()(const PureF<A>& fa)
    {
        std::function<B(A)> f = fTemplate;
        result = Parser<B> { PureF<B> { f(fa.ret) }, "" };
    }

    void operator()(const FreeF<A>& fa)
    {
        std::function<B(A)> f = fTemplate;
        std::function<Parser<B>(Parser<A>)> f2 =
            [=](const Parser<A> &pslInt)
        {
          return runFMap<A, B>(f, pslInt);
        };

        ParserMethods<Parser<B>, Parser> visited =
          fmapMethods(f2, fa.psf);
        result = Parser<B> { FreeF<B> { visited }, "" };
    }
};

} // namespace free
} // namespace core
} // namespace ps

#endif // PS_CORE_FREE_FUNCTOR_H
