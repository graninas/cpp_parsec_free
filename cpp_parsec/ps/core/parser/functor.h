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
        fb.next = [=](const Char d, Pos pos)
        {
            A faResult = fa.next(d, pos);
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
        fb.next = [=](const std::string& d, Pos pos)
        {
            A faResult = fa.next(d, pos);
            B gResult = g(faResult);
            return gResult;
        };
        result.psf = fb;
    }

    void operator()(const GetSt<A> &fa)
    {
      MapFunc<A, B> g = fTemplate;
      GetSt<B> fb;
      fb.next = [=](const State &st, Pos pos)
      {
        A faResult = fa.next(st, pos);
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
        fb.next = [=](const Unit& unit, Pos pos)
        {
            A faResult = fa.next(unit, pos);
            B gResult = g(faResult);
            return gResult;
        };
        result.psf = fb;
    }
};

template <typename A, typename B>
ParserADT<B> fmap(const MapFunc<A, B>& f,
                  const ParserADT<A>& method)
{
    ParserADTVisitor<A, B> visitor(f);
    std::visit(visitor, method.psf);
    return visitor.result;
}


} // namespace core
} // namespace ps

#endif // PS_CORE_PARSER_FUNCTOR_H
