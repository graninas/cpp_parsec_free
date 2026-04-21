#ifndef PS_CORE_METHODS_FUNCTOR_H
#define PS_CORE_METHODS_FUNCTOR_H

#include "../types.h"
#include "adt.h"

namespace ps
{
namespace core
{

template <typename A, typename B>
using MapFunc = std::function<B(A)>;

// Methods functor

template <typename A, typename B, template <typename> class Parser>
struct ParserMethodsVisitor
{
    MapFunc<A, B> fTemplate;
    ParserMethods<B, Parser> result;

    ParserMethodsVisitor(const MapFunc<A, B>& func)
        : fTemplate(func)
    {}

    void operator()(const ParseSymbolCond<A, Parser>& fa)
    {
      MapFunc<A, B> g = fTemplate;
      ParseSymbolCond<B, Parser> fb;
      fb.validator = fa.validator;
      fb.next = [=](const Any& d)
      {
        A faResult = fa.next(d);
        B gResult = g(faResult);
        return gResult;
      };
      result.psf = fb;
    }

    void operator()(const ParseLit<A, Parser>& fa)
    {
        MapFunc<A, B> g = fTemplate;
        ParseLit<B, Parser> fb;
        fb.s = fa.s;
        fb.next = [=](const std::string& d)
        {
            A faResult = fa.next(d);
            B gResult = g(faResult);
            return gResult;
        };
        result.psf = fb;
    }

    void operator()(const ParseMany<A, Parser>& fa)
    {
        MapFunc<A, B> g = fTemplate;
        ParseMany<B, Parser> fb;
        fb.rawParser = fa.rawParser;
        fb.next = [=](const std::list<Any>& d)
        {
            A faResult = fa.next(d);
            B gResult = g(faResult);
            return gResult;
        };
        result.psf = fb;
    }

    void operator()(const TryOrErrorParser<A, Parser> &fa)
    {
        MapFunc<A, B> g = fTemplate;
        TryOrErrorParser<B, Parser> fb;
        fb.rawParser = fa.rawParser;   // keep the same raw parser
        fb.next = [=](const ParserResult<Any>& d)
        {
            A faResult = fa.next(d);
            B gResult = g(faResult);
            return gResult;
        };
        result.psf = fb;
    }

    void operator()(const AltParser<A, Parser> &fa)
    {
        MapFunc<A, B> g = fTemplate;
        AltParser<B, Parser> fb;
        fb.p = fa.p;
        fb.q = fa.q;
        fb.next = [=](const Any& d)
        {
            A faResult = fa.next(d);
            B gResult = g(faResult);
            return gResult;
        };
        result.psf = fb;
    }

    void operator()(const LazyParser<A, Parser> &fa)
    {
        MapFunc<A, B> g = fTemplate;
        LazyParser<B, Parser> fb;
        fb.parserFactory = fa.parserFactory;   // keep the same factory
        fb.next = [=](const Any& d)
        {
            A faResult = fa.next(d);
            B gResult = g(faResult);
            return gResult;
        };
        result.psf = fb;
    }

    void operator()(const GetSt<A, Parser> &fa)
    {
      MapFunc<A, B> g = fTemplate;
      GetSt<B, Parser> fb;
      fb.next = [=](const State &st)
      {
        A faResult = fa.next(st);
        B gResult = g(faResult);
        return gResult;
      };
      result.psf = fb;
    }

    void operator()(const PutSt<A, Parser>& fa)
    {
        MapFunc<A, B> g = fTemplate;
        PutSt<B, Parser> fb;
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

template <typename A, typename B, template <typename> class Parser>
ParserMethods<B, Parser> fmapMethods(
    const MapFunc<A, B> &f,
    const ParserMethods<A, Parser> &method)
{
    ParserMethodsVisitor<A, B, Parser> visitor(f);
    std::visit(visitor, method.psf);
    return visitor.result;
}


} // namespace core
} // namespace ps

#endif // PS_CORE_METHODS_FUNCTOR_H
