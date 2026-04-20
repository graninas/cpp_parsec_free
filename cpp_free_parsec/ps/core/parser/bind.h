#ifndef PS_CORE_PARSER_BIND_H
#define PS_CORE_PARSER_BIND_H

#include "adt.h"
#include "functor.h"

namespace ps
{
namespace core
{

// forward declaration for bind
template <typename A, typename B>
struct BindParserVisitor;

template <typename A, typename B>
Parser<B> runBind(const Parser<A>& psl,
                   const std::function<Parser<B>(A)>& f)
{
    BindParserVisitor<A, B> visitor(f);
    std::visit(visitor, psl.psl);
    return visitor.newParserFreeADT;
}

template <typename A, typename B>
struct BindParserADTVisitor
{
    std::function<Parser<B>(A)> fTemplate;
    ParserADT<Parser<B>> newParserADT;

    BindParserADTVisitor(const std::function<Parser<B>(A)>& func)
        : fTemplate(func)
    {}

    void operator()(const ParseSymbolCond<Parser<A>>& fa)
    {
        std::function<Parser<B>(A)> g = fTemplate;

        ParseSymbolCond<Parser<B>> fb;
        fb.validator = fa.validator;
        fb.next = [=](const Any& d)
        {
            Parser<A> intermediate = fa.next(d);
            return runBind<A, B>(intermediate, g);
        };
        newParserADT.psf = fb;
    }

    void operator()(const ParseLit<Parser<A>>& fa)
    {
        std::function<Parser<B>(A)> g = fTemplate;

        ParseLit<Parser<B>> fb;
        fb.s = fa.s;
        fb.next = [=](const std::string& d)
        {
            Parser<A> intermediate = fa.next(d);
            return runBind<A, B>(intermediate, g);
        };
        newParserADT.psf = fb;
    }

      void operator()(const ParseMany<Parser<A>>& fa)
      {
          std::function<Parser<B>(A)> g = fTemplate;
          ParseMany<Parser<B>> fb;
          fb.rawParser = fa.rawParser;   // keep the same raw parser
          fb.next = [=](const std::list<Any>& d)
          {
              Parser<A> intermediate = fa.next(d);
              return runBind<A, B>(intermediate, g);
          };
          newParserADT.psf = fb;
      }

      void operator()(const TryOrErrorParser<Parser<A>>& fa)
      {
          std::function<Parser<B>(A)> g = fTemplate;
          TryOrErrorParser<Parser<B>> fb;
          fb.rawParser = fa.rawParser;   // keep the same raw parser
          fb.next = [=](const ParserResult<Any> &d)
          {
              Parser<A> intermediate = fa.next(d);
              return runBind<A, B>(intermediate, g);
          };
          newParserADT.psf = fb;
      }

      void operator()(const AltParser<Parser<A>>& fa)
      {
          std::function<Parser<B>(A)> g = fTemplate;
          AltParser<Parser<B>> fb;
          fb.p = fa.p;
          fb.q = fa.q;
          fb.next = [=](const Any& d)
          {
              Parser<A> intermediate = fa.next(d);
              return runBind<A, B>(intermediate, g);
          };
          newParserADT.psf = fb;
      }

      void operator()(const LazyParser<Parser<A>>& fa)
      {
          std::function<Parser<B>(A)> g = fTemplate;
          LazyParser<Parser<B>> fb;
          fb.parserFactory = fa.parserFactory;   // keep the same factory
          fb.next = [=](const Any& d)
          {
              Parser<A> intermediate = fa.next(d);
              return runBind<A, B>(intermediate, g);
          };
          newParserADT.psf = fb;
      }

      void operator()(const GetSt<Parser<A>>& fa)
      {
          std::function<Parser<B>(A)> g = fTemplate;
          GetSt<Parser<B>> fb;
          fb.next = [=](const State& st)
          {
              Parser<A> intermediate = fa.next(st);
              return runBind<A, B>(intermediate, g);
          };
          newParserADT.psf = fb;
      }

      void operator()(const PutSt<Parser<A>>& fa)
      {
          std::function<Parser<B>(A)> g = fTemplate;
          PutSt<Parser<B>> fb;
          fb.st = fa.st;
          fb.next = [=](const Unit& unit)
          {
              Parser<A> intermediate = fa.next(unit);
              return runBind<A, B>(intermediate, g);
          };
          newParserADT.psf = fb;
      }

};

template <typename A, typename B>
struct BindParserVisitor
{
    std::function<Parser<B>(A)> fTemplate;
    Parser<B> newParserFreeADT;

    BindParserVisitor(const std::function<Parser<B>(A)>& func)
        : fTemplate(func)
    {}

    void operator()(const PureF<A>& fa)
    {
        std::function<Parser<B>(A)> f = fTemplate;
        newParserFreeADT = f(fa.ret);
        newParserFreeADT.debugInfo = "";
    }

    void operator()(const FreeF<A>& fa)
    {
        std::function<Parser<B>(A)> f = fTemplate;
        BindParserADTVisitor<A, B> visitor(f);
        std::visit(visitor, fa.psf.psf);
        newParserFreeADT = { FreeF<B>{ visitor.newParserADT }, "" };
    }
};

template <typename A, typename B>
Parser<B> bind(const Parser<A>& ma,
                const std::function<Parser<B>(A)>& f)
{
    return runBind<A, B>(ma, f);
}


} // namespace core
} // namespace ps

#endif // PS_CORE_PARSER_BIND_H
