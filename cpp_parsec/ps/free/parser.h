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

namespace ps
{
namespace free
{

template <typename A>
using PL = ParserL<A>;

template <typename A>
using ParserT = ParserLST<PL, A>;

// ParserT generic monadic interface.

//template <typename A, typename B>
//ParserT<B> bind(const ParserT<A>& ma,
//                const std::function<ParserT<B>(A)>& f)
//{
//    BindParserLSTVisitor<PL, A, B> visitor(f);
//    std::visit(visitor, ma.psl);
//    return visitor.result;
//}

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

//template <typename A,
//          template <typename, typename> class Method>
//static ParserT<A> wrap(const Method<PL, ParserT<A>>& method)
//{
//    return { FreeFST<PL, A> {
//            psfst::ParserFST<PL, ParserT<A>> { method } } };
//}

template <typename A>
ParserT<A> pure(const A& a)
{
    return { PureFST<PL, A>{ a } };
}


template <typename A>
ParserT<ParseResult<A>> tryP(
        const PL<A>& parser)
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
//    return wrap(r);

    auto r2 = psfst::ParserFST<PL, ParserT<ParseResult<A>>> { r };
    return { FreeFST<PL, ParseResult<A>> { r2 } };
}


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


const ParserT<ParseResult<Char>> digit = tryP<Char>(digitPL);
const ParserT<ParseResult<Char>> lower = tryP<Char>(lowerPL);
const ParserT<ParseResult<Char>> upper    = tryP<Char>(upperPL);
const ParserT<ParseResult<Char>> letter   = tryP<Char>(letterPL);
const ParserT<ParseResult<Char>> alphaNum = tryP<Char>(alphaNumPL);

const auto symbol = [](Char ch) {
    return tryP<Char>(symbolPL(ch));
};


/// ParserL evaluation

template <typename A>
ParseResult<A> parseP(
        const ParserT<A>& pst,
        const std::string& s)
{
    if (s.empty())
        return { ParseError { "Source string is empty." }};

    ParserRuntime runtime(s, State {0});
    ParseResult<A> res = runParserT<PL, A>(runtime, pst);
    return res;
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

//template <typename T>
//ParserL<T> alt(const ParserL<T>& l, const ParserL<T>& r)
//{
//    std::function<ParserL<T>(T)> f = [](T t) { return pure<T>(t); };
//    return bindT<T, T>(l, r, f);
//}


} // namespace free
} // namespace ps

#endif // PS_FREE_PS_H
