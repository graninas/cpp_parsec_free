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
struct BindParserLVisitor;

template <typename A, typename B>
ParserL<B> runBind(const ParserL<A>& psl,
                   const std::function<ParserL<B>(A)>& f)
{
    BindParserLVisitor<A, B> visitor(f);
    std::visit(visitor, psl.psl);
    return visitor.newParserFreeADT;
}

template <typename A, typename B>
struct BindParserADTVisitor
{
    std::function<ParserL<B>(A)> fTemplate;
    ParserADT<ParserL<B>> newParserADT;

    BindParserADTVisitor(const std::function<ParserL<B>(A)>& func)
        : fTemplate(func)
    {}

    void operator()(const ParseSymbolCond<ParserL<A>>& fa)
    {
        std::function<ParserL<B>(A)> g = fTemplate;

        ParseSymbolCond<ParserL<B>> fb;
        fb.name = fa.name;
        fb.validator = fa.validator;
        fb.next = [=](const Any& d)
        {
            ParserL<A> intermediate = fa.next(d);
            return runBind<A, B>(intermediate, g);
        };
        newParserADT.psf = fb;
    }

    void operator()(const ParseLit<ParserL<A>>& fa)
    {
        std::function<ParserL<B>(A)> g = fTemplate;

        ParseLit<ParserL<B>> fb;
        fb.s = fa.s;
        fb.next = [=](const std::string& d)
        {
            ParserL<A> intermediate = fa.next(d);
            return runBind<A, B>(intermediate, g);
        };
        newParserADT.psf = fb;
    }

      void operator()(const ParseMany<ParserL<A>>& fa)
      {
          std::function<ParserL<B>(A)> g = fTemplate;
          ParseMany<ParserL<B>> fb;
          fb.raw_parser = fa.raw_parser;   // keep the same raw parser
          fb.next = [=](const std::list<Any>& d)
          {
              ParserL<A> intermediate = fa.next(d);
              return runBind<A, B>(intermediate, g);
          };
          newParserADT.psf = fb;
      }

      void operator()(const GetSt<ParserL<A>>& fa)
      {
          std::function<ParserL<B>(A)> g = fTemplate;
          GetSt<ParserL<B>> fb;
          fb.next = [=](const State& st)
          {
              ParserL<A> intermediate = fa.next(st);
              return runBind<A, B>(intermediate, g);
          };
          newParserADT.psf = fb;
      }

      void operator()(const PutSt<ParserL<A>>& fa)
      {
          std::function<ParserL<B>(A)> g = fTemplate;
          PutSt<ParserL<B>> fb;
          fb.st = fa.st;
          fb.next = [=](const Unit& unit)
          {
              ParserL<A> intermediate = fa.next(unit);
              return runBind<A, B>(intermediate, g);
          };
          newParserADT.psf = fb;
      }

};

template <typename A, typename B>
struct BindParserLVisitor
{
    std::function<ParserL<B>(A)> fTemplate;
    ParserL<B> newParserFreeADT;

    BindParserLVisitor(const std::function<ParserL<B>(A)>& func)
        : fTemplate(func)
    {}

    void operator()(const PureF<A>& fa)
    {
        std::function<ParserL<B>(A)> f = fTemplate;
        newParserFreeADT = f(fa.ret);
    }

    void operator()(const FreeF<A>& fa)
    {
        std::function<ParserL<B>(A)> f = fTemplate;
        BindParserADTVisitor<A, B> visitor(f);
        std::visit(visitor, fa.psf.psf);
        newParserFreeADT = { FreeF<B>{ visitor.newParserADT } };
    }
};

template <typename A, typename B>
ParserL<B> bind(const ParserL<A>& ma,
                const std::function<ParserL<B>(A)>& f)
{
    return runBind<A, B>(ma, f);
}


} // namespace core
} // namespace ps

#endif // PS_CORE_PARSER_BIND_H
