#ifndef PS_FREE_BINDST_H
#define PS_FREE_BINDST_H

#include "parserlst.h"
#include "../psf/parserfst.h"

namespace ps
{
namespace free
{

// forward declaration for bind
template <template <typename> class P,
          typename A,
          typename B>
struct BindParserLSTVisitor;

template <template <typename> class P,
          typename A,
          typename B>
using ArrowFuncST = std::function<ParserLST<P, B>(A)>;

template <template <typename> class P,
          typename A,
          typename B>
ParserLST<P, B> runBindST(
        const ParserLST<P, A>& pslst,
        const ArrowFuncST<P, A, B>& f)
{
    BindParserLSTVisitor<P, A, B> visitor(f);
    std::visit(visitor, pslst.pslst);
    return visitor.result;
}


template <template <typename> class P,
          typename A,
          typename B>
struct BindParserFSTVisitor
{
    ArrowFuncST<P, A, B> fTemplate;
    psfst::ParserFST<P, ParserLST<P, B>> result;

    BindParserFSTVisitor(const ArrowFuncST<P, A, B>& func)
        : fTemplate(func)
    {}

    void operator()(const psfst::TryPA<P, ParserLST<P, A>>& fa)
    {
        ArrowFuncST<P, A, B> f = fTemplate;
        psfst::TryPA<P, ParserLST<P, B>> fb;
        fb.parser = fa.parser;
        fb.next = [=](const ParseResult<Any>& result)
        {
            ParserLST<P, A> nextA = fa.next(result);
            return runBindST<P, A, B>(nextA, f);
        };
        result.psfst = fb;
    }

    void operator()(const psfst::EvalPA<P, ParserLST<P, A>>& fa)
    {
        ArrowFuncST<P, A, B> f = fTemplate;
        psfst::EvalPA<P, ParserLST<P, B>> fb;
        fb.parser = fa.parser;
        fb.next = [=](const ParseResult<Any>& result)
        {
            ParserLST<P, A> nextA = fa.next(result);
            return runBindST<P, A, B>(nextA, f);
        };
        result.psfst = fb;
    }
};

template <template <typename> class P,
          typename A,
          typename B>
struct BindParserLSTVisitor
{
    ArrowFuncST<P, A, B> fTemplate;
    ParserLST<P, B> result;

    BindParserLSTVisitor(const ArrowFuncST<P, A, B>& func)
        : fTemplate(func)
    {}

    void operator()(const PureFST<P, A>& fa)
    {
        ArrowFuncST<P, A, B> f = fTemplate;
        result = f(fa.ret);
    }

    void operator()(const FreeFST<P, A>& fa)
    {
        ArrowFuncST<P, A, B> f = fTemplate;
        BindParserFSTVisitor<P, A, B> visitor(f);
        std::visit(visitor, fa.psfst.psfst);
        psfst::ParserFST<P, ParserLST<P, B>> visited
                = visitor.result;
        result.pslst = FreeFST<P, B> { visited };
    }
};

} // namespace free
} // namespace ps

#endif // PS_FREE_BINDST_H
