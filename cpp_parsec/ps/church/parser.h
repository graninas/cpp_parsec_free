#ifndef PS_CHURCH_PS_H
#define PS_CHURCH_PS_H

#include "../types.h"
#include "../context.h"
#include "../impl/runtime.h"
#include "parserl.h"
#include "interpreter.h"

namespace ps
{
namespace church
{

template <typename A, typename B>
ParserL<B> orElseBind(const ParserL<A>& ma,
                      const ParserL<A>& mOnFail,
                      const std::function<ParserL<B>(A)>& onSuccess)
{
    ParserL<B> n;
    n.runF = [=](const std::function<PRA(B)>& p,
                 const std::function<PRA(psf::ParserF<PRA>)>& r)
    {
        std::function<PRA(A)> fst = [=](const A& a)
        {
            ParserL<B> internal = onSuccess(a);
            PRA res = internal.runF(p, r);
            return res;
        };

        PRA runFResult = ma.runF(fst, r);
        if (isLeft(runFResult))
        {
            runFResult = mOnFail.runF(fst, r);
        }
        return runFResult;
    };

    return n;
}

template <typename A, typename B>
ParserL<B> bind(const ParserL<A>& ma,
                const std::function<ParserL<B>(A)>& f)
{
    ParserL<B> n;
    n.runF = [=](const std::function<PRA(B)>& p,
                 const std::function<PRA(psf::ParserF<PRA>)>& r)
    {
        std::function<PRA(A)> fst = [=](const A& a)
        {
            ParserL<B> internal = f(a);
            PRA res = internal.runF(p, r);
            return res;
        };

        PRA runFResult = ma.runF(fst, r);
        return runFResult;
    };

    return n;
}

template <typename A>
ParserL<A> join(const ParserL<ParserL<A>>& mma)
{
    return bind<ParserL<A>, A>(mma, [](const ParserL<A>& ma) { return ma; });
}

template <typename A>
ParserL<A> pure(const A& a, const std::string& name = "")
{
    ParserL<A> n;
    n.runF = [=](const std::function<PRA(A)>& p,
                 const std::function<PRA(psf::ParserF<PRA>)>&)
    {
        PRA pResult = p(a);
        return pResult;
    };
    return n;
}

template <typename A, template <typename> class Method>
ParserL<A> wrap(const Method<A>& method, const std::string& name = "")
{
    ParserL<A> n;

    n.runF = [=](const std::function<PRA(A)>& p,
                 const std::function<PRA(psf::ParserF<PRA>)>& r)
    {
        psf::ParserF<A> f { method };
        psf::ParserF<PRA> mapped = psf::fmap<A, PRA>(p, f);
        PRA rResult = r(mapped);
        return rResult;
    };

    return n;
}

ParserL<Digit> parseDigit()
{
    return wrap(psf::ParseDigit<Digit>{ id }, "[ParseDigit]");
}

ParserL<Char> parseLowerCaseChar()
{
    return wrap(psf::ParseLowerCaseChar<Char>{ id }, "[ParseLowerCaseChar]");
}

ParserL<Char> parseUpperCaseChar()
{
    return wrap(psf::ParseUpperCaseChar<Char>{ id }, "[ParseUpperCaseChar]");
}

ParserL<Char> parseSymbol(Char ch)
{
    return wrap(psf::ParseSymbol<Char>{ ch, id },
                std::string("[ParseSymbol: ") + ch + "]");
}

const ParserL<Digit> digit = parseDigit();
const ParserL<Char> lowerCaseChar = parseLowerCaseChar();
const ParserL<Char> upperCaseChar = parseUpperCaseChar();
const auto symbol = [](Char ch) { return parseSymbol(ch); };

/// ParserL evaluation

template <typename A>
ps::ParseResult<A> parse(
        const ParserL<A>& psl,
        const std::string& s)
{
    if (s.empty())
        return { ParseError { "Source string is empty." }};

    ParserRuntime runtime(s, 0);
    return runParserL<A>(runtime, psl);
}

//template <typename T>
//ParserL<Many<T>> many(const ParserL<T>& parser)
//{

//}

template <typename T>
ParserL<T> alt(const ParserL<T>& l, const ParserL<T>& r)
{
    std::function<ParserL<T>(T)> f = [](T t) { return pure<T>(t); };
    return orElseBind<T, T>(l, r, f);
}



} // namespace church
} // namespace ps

#endif // PS_CHURCH_PS_H
