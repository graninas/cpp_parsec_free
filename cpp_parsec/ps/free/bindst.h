#ifndef PS_FREE_BINDST_H
#define PS_FREE_BINDST_H

#include "parserlst.h"
#include "../psf/parserfst.h"

namespace ps
{
namespace free
{

// forward declaration for bind
template <typename A, typename B>
struct BindParserLSTVisitor;

template <typename A, typename B>
using ArrowFuncST = std::function<ParserLST<B>(A)>;

template <typename A, typename B>
ParserLST<B> runBindST(
        const ParserLST<A>& pslst,
        const ArrowFuncST<A, B>& f)
{
    BindParserLSTVisitor<A, B> visitor(f);
    std::visit(visitor, pslst.pslst);
    return visitor.result;
}


template <typename A, typename B>
struct BindParserFSTVisitor
{
    ArrowFuncST<A, B> fTemplate;
    psfst::ParserFST<ParserLST<B>> result;

    BindParserFSTVisitor(const ArrowFuncST<A, B>& func)
        : fTemplate(func)
    {}

    void operator()(const psfst::SafePA<ParserLST<A>>& fa)
    {
        ArrowFuncST<A, B> f = fTemplate;
        psfst::SafePA<ParserLST<B>> fb;
        fb.parser = fa.parser;
        fb.next = [=](const ParserResult<Any>& result)
        {
            ParserLST<A> nextA = fa.next(result);
            return runBindST<A, B>(nextA, f);
        };
        result.psfst = fb;
    }

    void operator()(const psfst::TryPA<ParserLST<A>>& fa)
    {
        ArrowFuncST<A, B> f = fTemplate;
        psfst::TryPA<ParserLST<B>> fb;
        fb.parser = fa.parser;
        fb.next = [=](const ParserResult<Any>& result)
        {
            ParserLST<A> nextA = fa.next(result);
            return runBindST<A, B>(nextA, f);
        };
        result.psfst = fb;
    }

    void operator()(const psfst::EvalPA<ParserLST<A>>& fa)
    {
        ArrowFuncST<A, B> f = fTemplate;
        psfst::EvalPA<ParserLST<B>> fb;
        fb.parser = fa.parser;
        fb.next = [=](const ParserResult<Any>& result)
        {
            ParserLST<A> nextA = fa.next(result);
            return runBindST<A, B>(nextA, f);
        };
        result.psfst = fb;
    }
};

template <typename A, typename B>
struct BindParserLSTVisitor
{
    ArrowFuncST<A, B> fTemplate;
    ParserLST<B> result;

    BindParserLSTVisitor(const ArrowFuncST<A, B>& func)
        : fTemplate(func)
    {}

    void operator()(const PureFST<A>& fa)
    {
        ArrowFuncST<A, B> f = fTemplate;
        result = f(fa.ret);
    }

    void operator()(const FreeFST<A>& fa)
    {
        ArrowFuncST<A, B> f = fTemplate;
        BindParserFSTVisitor<A, B> visitor(f);
        std::visit(visitor, fa.psfst.psfst);
        psfst::ParserFST<ParserLST<B>> visited
                = visitor.result;
        result.pslst = FreeFST<B> { visited };
    }
};

} // namespace free
} // namespace ps

#endif // PS_FREE_BINDST_H
