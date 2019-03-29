#ifndef PS_PSF_FUNCTOR_H
#define PS_PSF_FUNCTOR_H

#include "psf.h"

namespace ps
{
namespace psf
{

template <typename A, typename B>
using MapFunc = std::function<B(A)>;

template <typename A, typename B>
struct StmfFunctorVisitor
{
    MapFunc<A,B> fTemplate;
    PSF<B> result;

    StmfFunctorVisitor(const MapFunc<A,B>& func)
        : fTemplate(func)
    {}

    void operator()(const NewTVarA<A>& fa)
    {
        MapFunc<A,B> g = fTemplate;
        NewTVarA<B> fb;
        fb.val = fa.val;
        fb.name = fa.name;
        fb.next = [=](const TVarAny& tvar)
        {
            return g(fa.next(tvar));
        };
        result.psf = fb;
    }

    void operator()(const ReadTVarA<A>& fa)
    {
        MapFunc<A,B> g = fTemplate;
        ReadTVarA<B> fb;
        fb.tvar = fa.tvar;
        fb.next = [=](const Any& val)
        {
            return g(fa.next(val));
        };
        result.psf = fb;
    }

    void operator()(const WriteTVarA<A>& fa)
    {
        MapFunc<A,B> g = fTemplate;
        WriteTVarA<B> fb;
        fb.tvar = fa.tvar;
        fb.val = fa.val;
        fb.next = [=](const Unit& u)
        {
            return g(fa.next(u));
        };
        result.psf = fb;
    }

    void operator()(const RetryA<A>&)
    {
        result.psf = RetryA<B> {};
    }
};

template <typename A, typename B>
PSF<B> fmap(const MapFunc<A, B>& f,
             const PSF<A>& method)
{
    StmfFunctorVisitor<A, B> visitor(f);
    std::visit(visitor, method.psf);
    return visitor.result;
}

} // namespace psf
} // namespace ps

#endif // PS_PSF_FUNCTOR_H
