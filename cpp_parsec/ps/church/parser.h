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
ParserL<B> bindT(const ParserL<A>& ma,
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
ParserL<A> pure(const A& a)
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
ParserL<A> wrap(const Method<A>& method)
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

ParserL<Char> parseSymbolCond(
        const std::string& name,
        const std::function<bool(char)>& validator)
{
    return wrap(psf::ParseSymbolCond<Char>{ name, validator, id });
}

std::function<bool(char)> chEq(char ch)
{
    return [=](char ch1) { return ch1 == ch; };
}

const auto isLower = [](char ch)
{
    return ch >= 'a' && ch <= 'z';
};

const auto isUpper = [](char ch)
{
    return ch >= 'A' && ch <= 'Z';
};

const auto isAlpha = [](char ch)
{
    return isLower(ch) || isUpper(ch);
};

const auto isDigit = [](char ch)
{
    return ch >= '0' && ch <= '9';
};

const auto isAlphaNum = [](char ch)
{
    return isAlpha(ch) || isDigit(ch);
};

const ParserL<Char> digit    = parseSymbolCond("digit",    isDigit);
const ParserL<Char> lower    = parseSymbolCond("lower",    isLower);
const ParserL<Char> upper    = parseSymbolCond("upper",    isUpper);
const ParserL<Char> letter   = parseSymbolCond("letter",   isAlpha);
const ParserL<Char> alphaNum = parseSymbolCond("alphaNum", isAlphaNum);

const auto symbol = [](Char ch) {
    return parseSymbolCond(std::string() + ch, chEq(ch));
};

/// ParserL evaluation

template <typename A>
ps::ParserResult<A> parse(
        const ParserL<A>& psl,
        const std::string& s)
{
    if (s.empty())
        return { ParserFailed { "Source string is empty." }};

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
    return bindT<T, T>(l, r, f);
}

} // namespace church
} // namespace ps

#endif // PS_CHURCH_PS_H
