#ifndef PS_FREE_BIND_H
#define PS_FREE_BIND_H

#include "parserl.h"
#include "../psf/parserf.h"

namespace ps
{
namespace free
{

// forward declaration for bind
template <typename A, typename B>
struct BindParserLVisitor;

template <typename A, typename B>
ParserL<B> runBind(const ParserL<A>& psl,
                   const std::function<ParserL<B>(A)>& f)
{
    BindParserLVisitor<A, B> visitor(f);
    std::visit(visitor, psl.psl);
    return visitor.result;
}

template <typename A, typename B>
struct BindParserFVisitor
{
    std::function<ParserL<B>(A)> fTemplate;
    psf::ParserF<ParserL<B>> result;

    BindParserFVisitor(const std::function<ParserL<B>(A)>& func)
        : fTemplate(func)
    {}

    void operator()(const psf::ParseSymbolCond<ParserL<A>>& fa)
    {
        std::function<ParserL<B>(A)> f = fTemplate;
        psf::ParseSymbolCond<ParserL<B>> fb;
        fb.validator = fa.validator;
        fb.name = fa.name;
        fb.next = [=](const ParserResult<Char>& ch)
        {
            ParserL<A> nextA = fa.next(ch);
            return runBind<A, B>(nextA, f);
        };
        result.psf = fb;
    }

    void operator()(const psf::GetSt<ParserL<A>>&)
    {
        throw std::runtime_error("GetSt bind not implemented.");
    }

    void operator()(const psf::PutSt<ParserL<A>>&)
    {
        throw std::runtime_error("PutSt bind not implemented.");
    }
};

template <typename A, typename B>
struct BindParserLVisitor
{
    std::function<ParserL<B>(A)> fTemplate;
    ParserL<B> result;

    BindParserLVisitor(const std::function<ParserL<B>(A)>& func)
        : fTemplate(func)
    {}

    void operator()(const PureF<A>& fa)
    {
        std::function<ParserL<B>(A)> f = fTemplate;
        result = f(fa.ret);
    }

    void operator()(const FreeF<A>& fa)
    {
        std::function<ParserL<B>(A)> f = fTemplate;
        BindParserFVisitor<A, B> visitor(f);
        std::visit(visitor, fa.psf.psf);
        psf::ParserF<ParserL<B>> visited = visitor.result;
        result.psl = FreeF<B> { visited };
    }
};

} // namespace free
} // namespace ps

#endif // PS_FREE_BIND_H
