#ifndef PS_FREE_PS_H
#define PS_FREE_PS_H

#include "../types.h"
#include "../context.h"
#include "parserl.h"
#include "parserlst.h"
#include "bind.h"
#include "bindst.h"
#include "interpreter.h"
#include "interpreterst.h"
#include "parserl_functor.h"
#include "../psf/visitorst.h"

namespace ps
{
namespace free
{

template <typename A>
using PL = ParserL<A>;

template <typename A>
using ParserT = ParserLST<PL, A>;

// ParserT generic monadic interface.

template <typename A, typename B>
ParserL<B> bindPL(const ParserL<A>& ma,
                  const std::function<ParserL<B>(A)>& f)
{
    return runBind<A, B>(ma, f);
}

// Special hacky function, do not use it.
template <typename A, typename B>
ParserT<B> bindTry(
        const ParserT<ParseResult<A>>& ma,
        const ParserT<ParseResult<A>>& mOnFail,
        const std::function<ParserT<B>(A)>& f)
{
    std::function<ParserT<B>(ParseResult<A>)> f3 =
            [=](const ParseResult<A>& res)
    {
        if (isLeft(res))
        {
            throw std::runtime_error(getError(res).message);
        }

        return f(getParsed(res));
    };

    std::function<ParserT<B>(ParseResult<A>)> f2 =
            [=](const ParseResult<A>& res)
    {
        if (isLeft(res))
        {
            return runBindST(mOnFail, f3);
        }

        return f(getParsed(res));
    };

    return runBindST(ma, f2);
}

template <typename A, typename B>
ParserT<B> bind(
        const ParserT<A>& ma,
        const std::function<ParserT<B>(A)>& f)
{
    return runBindST(ma, f);
}

template <typename A,
          template <typename> class Method>
static ParserL<A> wrapPL(const Method<ParserL<A>>& method)
{
    return { FreeF<A> { psf::ParserF<ParserL<A>> { method } } };
}

template <typename A>
ParserL<A> purePL(const A& a)
{
    return { PureF<A>{ a } };
}

template <typename A>
ParserT<A> pure(const A& a)
{
    return { PureFST<PL, A>{ a } };
}

template <typename A>
ParserT<A> evalP(const PL<A>& parser)
{
    std::function<PL<Any>(PL<A>)> pToAny = [](const PL<A>& psl)
    {
        return fmap<A, Any>([](const A& a) { return a; }, psl);
    };

    psfst::EvalPA<PL, ParserT<ParseResult<A>>> r
        = psfst::EvalP<PL, A, ParserT<ParseResult<A>>>::toAny(
            parser,
            pToAny,
            [](const ParseResult<A>& pr)
                { return pure<ParseResult<A>>(pr); }
            );

    std::function<ParserLST<PL, A>(ParserLST<PL, ParseResult<A>>)> df
            = [](const ParserLST<PL, ParseResult<A>>& pslst)
    {
        ParserLST<PL, A> lstMapped = runBindST<PL, ParseResult<A>, A>(pslst, [](const ParseResult<A>& pr)
        {
            if (isLeft(pr))
            {
                throw std::runtime_error(getError(pr).message);
            }
            return pure<A>(getParsed(pr));
        });

        return lstMapped;
    };
    psfst::ParserFST<PL, ParserT<ParseResult<A>>> r2 = { r };
    auto rMapped = psfst::fmap<PL, ParserLST<PL, ParseResult<A>>, ParserLST<PL, A>>(df, r2);
    return ParserT<A> { FreeFST<PL, A> { rMapped } };
}

template <typename A>
ParserT<ParseResult<A>> tryP(const PL<A>& parser)
{
    std::function<PL<Any>(PL<A>)> pToAny = [](const PL<A>& psl)
    {
        return fmap<A, Any>([](const A& a) { return a; }, psl);
    };

    psfst::TryPA<PL, ParserT<ParseResult<A>>> r
        = psfst::TryP<PL, A, ParserT<ParseResult<A>>>::toAny(
            parser,
            pToAny,
            [](const ParseResult<A>& pr)
                { return pure<ParseResult<A>>(pr); }
            );

    auto r2 = psfst::ParserFST<PL, ParserT<ParseResult<A>>> { r };
    return { FreeFST<PL, ParseResult<A>> { r2 } };
}

//ParserL<Unit> putStPL(const State& st)
//{
//    return wrapPL(psf::PutSt<ParserL<Unit>>{ st,
//                [](const Unit&) { return purePL<Unit>(unit); }
//                });
//}

//ParserL<State> getStPL()
//{
//    return wrapPL(psf::GetSt<ParserL<State>>{
//                [](const State& st) { return purePL<State>(st); }
//                });
//}

ParserL<Char> parseSymbolCond(
        const std::string& name,
        const std::function<bool(char)>& validator)
{
    return wrapPL(psf::ParseSymbolCond<ParserL<Char>>{ name, validator,
                [](Char ch) { return purePL<Char>(ch); }
                });
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

const ParserL<Char> digitPL    = parseSymbolCond("digit",    isDigit);
const ParserL<Char> lowerPL    = parseSymbolCond("lower",    isLower);
const ParserL<Char> upperPL    = parseSymbolCond("upper",    isUpper);
const ParserL<Char> letterPL   = parseSymbolCond("letter",   isAlpha);
const ParserL<Char> alphaNumPL = parseSymbolCond("alphaNum", isAlphaNum);

const auto symbolPL = [](Char ch) {
    return parseSymbolCond(std::string() + ch, chEq(ch));
};


const ParserT<ParseResult<Char>> digitP    = tryP<Char>(digitPL);
const ParserT<ParseResult<Char>> lowerP    = tryP<Char>(lowerPL);
const ParserT<ParseResult<Char>> upperP    = tryP<Char>(upperPL);
const ParserT<ParseResult<Char>> letterP   = tryP<Char>(letterPL);
const ParserT<ParseResult<Char>> alphaNumP = tryP<Char>(alphaNumPL);

const auto symbolP = [](Char ch) {
    return tryP<Char>(symbolPL(ch));
};

const ParserT<Char> digit    = evalP<Char>(digitPL);
const ParserT<Char> lower    = evalP<Char>(lowerPL);
const ParserT<Char> upper    = evalP<Char>(upperPL);
const ParserT<Char> letter   = evalP<Char>(letterPL);
const ParserT<Char> alphaNum = evalP<Char>(alphaNumPL);

const auto symbol = [](Char ch) {
    return evalP<Char>(symbolPL(ch));
};

/// ParserL evaluation

template <typename A>
ParseResult<A> parseP(
        const ParserT<A>& pst,
        const std::string& s)
{
    if (s.empty())
        return { ParseError { "Source string is empty." }};

    try
    {
        ParserRuntime runtime(s, State {0});
        ParseResult<A> res = runParserT<PL, A>(runtime, pst);
        return res;
    }
    catch (std::runtime_error err)
    {
        return ParseError { err.what() };
    }
}

template <typename A>
ParseResult<A> parse(
        const ParserT<ParseResult<A>>& pst,
        const std::string& s)
{
    ParseResult<ParseResult<A>> res = parseP(pst, s);
    if (isLeft(res))
    {
        auto pe = getError(res);
        return ParseError { pe.message };
    }
    else
    {
        auto se = getParsed(res);
        return se;
    }
}

template <typename A>
ParseResult<A> parse(
        const ParserT<A>& pst,
        const std::string& s)
{
    return parseP(pst, s);
}

template <typename A>
ParserT<A> alt(const ParserL<A>& l, const ParserL<A>& r)
{
    ParserT<ParseResult<A>> lp = tryP(l);
    ParserT<ParseResult<A>> rp = tryP(r);

    std::function<ParserT<A>(A)> f = [](const A& a) { return pure(a); };
    return bindTry(lp, rp, f);
}

} // namespace free
} // namespace ps

#endif // PS_FREE_PS_H
