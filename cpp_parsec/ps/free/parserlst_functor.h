//#ifndef PS_FREE_PARSERLST_FUNCTOR_H
//#define PS_FREE_PARSERLST_FUNCTOR_H

//#include "parserlst.h"
//#include "../psf/parserfst.h"

//namespace ps
//{
//namespace free
//{

//// forward declaration for bind
//template <template <typename> class P,
//          typename A,
//          typename B>
//struct FunctorParserLSTVisitor;

//template <template <typename> class P,
//          typename A,
//          typename B>
//using ArrowFuncSTF = std::function<B(A)>;

//template <template <typename> class P,
//          typename A,
//          typename B>
//ParserLST<P, B> runFunctorST(
//        const ParserLST<P, A>& pslst,
//        const ArrowFuncSTF<A, B>& f)
//{
//    FunctorParserLSTVisitor<P, A, B> visitor(f);
//    std::visit(visitor, pslst.pslst);
//    return visitor.result;
//}


//template <template <typename> class P,
//          typename A,
//          typename B>
//struct FunctorParserFSTVisitor
//{
//    ArrowFuncSTF<A, B> fTemplate;
//    psfst::ParserFST<P, ParserLST<P, B>> result;

//    FunctorParserFSTVisitor(const ArrowFuncSTF<A, B>& func)
//        : fTemplate(func)
//    {}

//    void operator()(const psfst::TryPA<P, ParserLST<P, A>>& fa)
//    {
//        ArrowFuncSTF<A, B> f = fTemplate;
//        psfst::TryPA<P, ParserLST<P, B>> fb;
//        fb.parser = fa.parser;
//        fb.next = [=](const ParseResult<Any>& result)
//        {
//            ParserLST<P, A> nextA = fa.next(result);
//            return runFunctorST<P, A, B>(nextA, f);
//        };
//        result.psfst = fb;
//    }

//    void operator()(const psfst::EvalPA<P, ParserLST<P, A>>& fa)
//    {
//        ArrowFuncSTF<A, B> f = fTemplate;
//        psfst::EvalPA<P, ParserLST<P, B>> fb;
//        fb.parser = fa.parser;
//        fb.next = [=](const ParseResult<Any>& result)
//        {
//            ParserLST<P, A> nextA = fa.next(result);
//            return runFunctorST<P, A, B>(nextA, f);
//        };
//        result.psfst = fb;
//    }
//};

//template <template <typename> class P,
//          typename A,
//          typename B>
//struct FunctorParserLSTVisitor
//{
//    ArrowFuncSTF<A, B> fTemplate;
//    ParserLST<P, B> result;

//    FunctorParserLSTVisitor(const ArrowFuncSTF<A, B>& func)
//        : fTemplate(func)
//    {}

//    void operator()(const PureFST<P, A>& fa)
//    {
//        ArrowFuncSTF<A, B> f = fTemplate;
//        result = f(fa.ret);
//    }

//    void operator()(const FreeFST<P, A>& fa)
//    {
//        ArrowFuncSTF<A, B> f = fTemplate;
//        FunctorParserFSTVisitor<P, A, B> visitor(f);
//        std::visit(visitor, fa.psfst.psfst);
//        psfst::ParserFST<P, ParserLST<P, B>> visited
//                = visitor.result;
//        result.pslst = FreeFST<P, B> { visited };
//    }
//};

//} // namespace free
//} // namespace ps

//#endif // PS_FREE_BINDST_H
