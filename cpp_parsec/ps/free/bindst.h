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
ParserLST<B> runBindST(const ParserLST<A>& pslst,
                       const ArrowFuncST<A, B>& f)
{
    BindParserLSTVisitor<A, B> visitor(f);
    std::visit(visitor, pslst.pslst);
    return visitor.result;
}


template <typename A, typename B>
struct BindParserFSTVisitor
{
    ArrowFuncST<A,B> fTemplate;
    psfst::ParserFST<ParserLST<B>> result;

    BindParserFSTVisitor(const ArrowFuncST<A,B>& func)
        : fTemplate(func)
    {}

    void operator()(const psfst::GetSt<ParserLST<A>>& fa)
    {
        ArrowFuncST<A,B> f = fTemplate;
        psfst::GetSt<ParserLST<B>> fb;
        fb.next = [=](const State& st)
        {
            ParserLST<A> nextA = fa.next(st);
            return runBindST<A, B>(nextA, f);
        };
        result.psfst = fb;
    }

    void operator()(const psfst::PutSt<ParserLST<A>>& fa)
    {
        ArrowFuncST<A,B> f = fTemplate;
        psfst::PutSt<ParserLST<B>> fb;
        fb.st = fa.st;
        fb.next = [=](const Unit)
        {
            ParserLST<A> nextA = fa.next(unit);
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

    BindParserLSTVisitor(const ArrowFuncST<A,B>& func)
        : fTemplate(func)
    {}

    void operator()(const PureFST<A>& fa)
    {
        ArrowFuncST<A,B> f = fTemplate;
        result = f(fa.ret);
    }

    void operator()(const FreeFST<A>& fa)
    {
        ArrowFuncST<A,B> f = fTemplate;
        BindParserFSTVisitor<A, B> visitor(f);
        std::visit(visitor, fa.psfst.psfst);
        psfst::ParserFST<ParserLST<B>> visited = visitor.result;
        result.pslst = FreeFST<B> { visited };
    }
};

} // namespace free
} // namespace ps

#endif // PS_FREE_BINDST_H
