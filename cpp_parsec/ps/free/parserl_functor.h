#ifndef PS_PARSERL_FUNCTOR_H
#define PS_PARSERL_FUNCTOR_H

#include "parserl.h"
#include "../psf/visitor.h"

namespace ps
{
namespace free
{

// Forward
template <typename A, typename B>
struct FunctorParserLVisitor;

template <typename A, typename B>
ParserL<B> fmap(
        const std::function<B(A)>& f,
        const ParserL<A>& psl)
{
    FunctorParserLVisitor<A, B> visitor(f);
    std::visit(visitor, psl.psl);
    return visitor.result;
}

template <typename A, typename B>
struct FunctorParserLVisitor
{
    std::function<B(A)> fTemplate;
    ParserL<B> result;

    FunctorParserLVisitor(const std::function<B(A)>& func)
        : fTemplate(func)
    {}

    void operator()(const PureF<A>& fa)
    {
        std::function<B(A)> f = fTemplate;
        result = ParserL<B> { PureF<B> { f(fa.ret) } };
    }

    void operator()(const FreeF<A>& fa)
    {
        std::function<B(A)> f = fTemplate;
        std::function<ParserL<B>(ParserL<A>)> f2 =
                [=](const ParserL<A>& pslInt)
        {
            return fmap<A, B>(f, pslInt);
        };

        psf::ParserF<ParserL<B>> visited = psf::fmap(f2, fa.psf);
        result = ParserL<B> { FreeF<B> { visited } };
    }
};

} // namespace free
} // namespace ps

#endif // PS_PARSERL_FUNCTOR_H
