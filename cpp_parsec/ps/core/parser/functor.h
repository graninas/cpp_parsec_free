#ifndef PS_CORE_PARSER_FUNCTOR_H
#define PS_CORE_PARSER_FUNCTOR_H

#include "../types.h"
#include "adt.h"

namespace ps
{
namespace core
{


template <typename A, typename B>
using MapFunc = std::function<B(A)>;

// Forward

template <typename A, typename B>
Parser<B> fmap(
    const std::function<B(A)> &f,
    const Parser<A> &psl);

    // Methods functor

    template <typename A, typename B>
    struct ParserADTVisitor
{
    MapFunc<A, B> fTemplate;
    ParserADT<B> result;

    ParserADTVisitor(const MapFunc<A, B>& func)
        : fTemplate(func)
    {}

    void operator()(const ParseSymbolCond<A>& fa)
    {
      MapFunc<A, B> g = fTemplate;
      ParseSymbolCond<B> fb;
      fb.name = fa.name;
      fb.validator = fa.validator;
      fb.next = [=](const Any& d)
      {
        A faResult = fa.next(d);
        B gResult = g(faResult);
        return gResult;
      };
      result.psf = fb;
    }

    void operator()(const ParseLit<A>& fa)
    {
        MapFunc<A, B> g = fTemplate;
        ParseLit<B> fb;
        fb.s = fa.s;
        fb.next = [=](const std::string& d)
        {
            A faResult = fa.next(d);
            B gResult = g(faResult);
            return gResult;
        };
        result.psf = fb;
    }

      void operator()(const ParseMany<A>& fa)
      {
          MapFunc<A, B> g = fTemplate;
          ParseMany<B> fb;
          fb.rawParser = fa.rawParser;
          fb.next = [=](const std::list<Any>& d)
          {
              A faResult = fa.next(d);
              B gResult = g(faResult);
              return gResult;
          };
          result.psf = fb;
      }

    void operator()(const GetSt<A> &fa)
    {
      MapFunc<A, B> g = fTemplate;
      GetSt<B> fb;
      fb.next = [=](const State &st)
      {
        A faResult = fa.next(st);
        B gResult = g(faResult);
        return gResult;
      };
      result.psf = fb;
    }

    void operator()(const PutSt<A>& fa)
    {
        MapFunc<A, B> g = fTemplate;
        PutSt<B> fb;
        fb.st = fa.st;
        fb.next = [=](const Unit& unit)
        {
            A faResult = fa.next(unit);
            B gResult = g(faResult);
            return gResult;
        };
        result.psf = fb;
    }

};

template <typename A, typename B>
ParserADT<B> fmapMethods(const MapFunc<A, B> &f,
                         const ParserADT<A> &method)
{
    ParserADTVisitor<A, B> visitor(f);
    std::visit(visitor, method.psf);
    return visitor.result;
}

// // Free functor

// Forward
template <typename A, typename B>
struct FunctorParserVisitor;

template <typename A, typename B>
Parser<B> fmap(
    const std::function<B(A)> &f,
    const Parser<A> &psl)
{
  FunctorParserVisitor<A, B> visitor(f);
  std::visit(visitor, psl.psl);
  return visitor.result;

  return {};
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
        result = Parser<B> { PureF<B> { f(fa.ret) } };
    }

    void operator()(const FreeF<A>& fa)
    {
        std::function<B(A)> f = fTemplate;
        std::function<Parser<B>(Parser<A>)> f2 =
                [=](const Parser<A>& pslInt)
        {
            return fmap<A, B>(f, pslInt);
        };

        ParserADT<Parser<B>> visited = fmapMethods(f2, fa.psf);
        result = Parser<B> { FreeF<B> { visited } };
    }


};

} // namespace core
} // namespace ps

#endif // PS_CORE_PARSER_FUNCTOR_H
