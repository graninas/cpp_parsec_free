#ifndef PS_PSF_H
#define PS_PSF_H

#include "parserf.h"

namespace ps
{
namespace psf
{

template <typename A, typename B>
using MapFunc = std::function<B(A)>;

template <typename A, typename B>
struct ParserFVisitor
{
    MapFunc<A, B> fTemplate;
    ParserF<B> result;

    ParserFVisitor(const MapFunc<A, B>& func)
        : fTemplate(func)
    {}

    void operator()(const ParseDigit<A>& fa)
    {
        std::cout << "ParserFVisitor::ParseDigit\n";

        MapFunc<A, B> g = fTemplate;
        ParseDigit<B> fb;
        fb.next = [=](const Digit d)
        {
            std::cout << "ParserFVisitor::ParseDigit -> \\fb.next -> fa.next()\n";
            auto faResult = fa.next(d);
            std::cout << "ParserFVisitor::ParseDigit -> \\fb.next -> g()\n";
            auto gResult = g(faResult);
            std::cout << "ParserFVisitor::ParseDigit -> \\fb.next -> success\n";
            return gResult;
        };
        result.psf = fb;
    }

    void operator()(const ParseUpperCaseChar<A>& fa)
    {
        MapFunc<A, B> g = fTemplate;
        ParseUpperCaseChar<B> fb;
        fb.next = [=](const Char d)
        {
            std::cout << "ParserFVisitor::ParseUCChar -> \\fb.next -> fa.next()\n";
            auto faResult = fa.next(d);
            std::cout << "ParserFVisitor::ParseUCChar -> \\fb.next -> g()\n";
            auto gResult = g(faResult);
            std::cout << "ParserFVisitor::ParseUCChar -> \\fb.next -> success\n";
            return gResult;
        };
        result.psf = fb;
    }
    void operator()(const ParseLowerCaseChar<A>& fa)
    {
        MapFunc<A, B> g = fTemplate;
        ParseLowerCaseChar<B> fb;
        fb.next = [=](const Char d)
        {
            std::cout << "ParserFVisitor::ParseLCChar -> \\fb.next -> fa.next()\n";
            auto faResult = fa.next(d);
            std::cout << "ParserFVisitor::ParseLCChar -> \\fb.next -> g()\n";
            auto gResult = g(faResult);
            std::cout << "ParserFVisitor::ParseLCChar -> \\fb.next -> success\n";
            return gResult;

        };
        result.psf = fb;
    }
};

template <typename A, typename B>
ParserF<B> fmap(const MapFunc<A, B>& f,
                const ParserF<A>& method)
{
    ParserFVisitor<A, B> visitor(f);
    std::visit(visitor, method.psf);
    return visitor.result;
}


} // namespace psf
} // namespace ps

#endif // PS_PSF_H
