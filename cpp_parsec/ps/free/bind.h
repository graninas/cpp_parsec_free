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
using ArrowFunc = std::function<ParserL<B>(A)>;

template <typename A, typename B,
          template <typename, typename> class Visitor>
ParserL<B> runBind(const ParserL<A>& psl, const ArrowFunc<A, B>& f)
{
    Visitor<A, B> visitor(f);
    std::visit(visitor, psl.psl);
    return visitor.result;
}


template <typename A, typename B>
struct BindStmfVisitor
{
    ArrowFunc<A,B> fTemplate;
    psf::ParserF<ParserL<B>> result;

    BindStmfVisitor(const ArrowFunc<A,B>& func)
        : fTemplate(func)
    {}

    void operator()(const psf::ParseSymbolCond<ParserL<A>>& fa)
    {
        ArrowFunc<A,B> f = fTemplate;
        psf::ParseSymbolCond<ParserL<B>> fb;
        fb.validator = fa.validator;
        fb.name = fa.name;
        fb.next = [=](Char ch)
        {
            ParserL<A> nextA = fa.next(ch);
            return runBind<A, B, BindParserLVisitor>(nextA, f);
        };
        result.psf = fb;
    }
};

template <typename A, typename B>
struct BindParserLVisitor
{
    ArrowFunc<A, B> fTemplate;
    ParserL<B> result;

    BindParserLVisitor(const ArrowFunc<A,B>& func)
        : fTemplate(func)
    {}

    void operator()(const PureF<A>& fa)
    {
        ArrowFunc<A,B> f = fTemplate;
        result = f(fa.ret);
    }

    void operator()(const FreeF<A>& fa)
    {
        ArrowFunc<A,B> f = fTemplate;
        BindStmfVisitor<A, B> visitor(f);
        std::visit(visitor, fa.psf.psf);
        psf::ParserF<ParserL<B>> visited = visitor.result;
        result.psl = FreeF<B> { visited };
    }
};

} // namespace free
} // namespace ps

#endif // PS_FREE_BIND_H
