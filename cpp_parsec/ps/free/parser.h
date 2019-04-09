#ifndef PS_FREE_PS_H
#define PS_FREE_PS_H

#include "../types.h"
#include "../context.h"
#include "parserl.h"
#include "bind.h"
#include "bindst.h"
#include "interpreter.h"
#include "interpreterst.h"

namespace ps
{
namespace free
{

// ParserL generic monadic interface.

template <typename A, typename B>
ParserL<B> bind(const ParserL<A>& ma,
                const std::function<ParserL<B>(A)>& f)
{
    BindParserLVisitor<A, B> visitor(f);
    std::visit(visitor, ma.psl);
    return visitor.result;
}

template <typename A, template <typename> class Method>
static ParserL<A> wrap(const Method<ParserL<A>>& method)
{
    return { FreeF<A> { psf::ParserF<ParserL<A>> { method } } };
}

template <typename A>
ParserL<A> pure(const A& a)
{
    return { PureF<A>{ a } };
}

ParserL<Char> parseSymbolCond(
        const std::string& name,
        const std::function<bool(char)>& validator)
{
    return wrap(psf::ParseSymbolCond<ParserL<Char>>{ name, validator,
                [](Char ch) { return pure<Char>(ch); }
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

const ParserL<Char> digit    = parseSymbolCond("digit",    isDigit);
const ParserL<Char> lower    = parseSymbolCond("lower",    isLower);
const ParserL<Char> upper    = parseSymbolCond("upper",    isUpper);
const ParserL<Char> letter   = parseSymbolCond("letter",   isAlpha);
const ParserL<Char> alphaNum = parseSymbolCond("alphaNum", isAlphaNum);

const auto symbol = [](Char ch) {
    return parseSymbolCond(std::string() + ch, chEq(ch));
};

//template <typename A>
//ParserL<Char> newTVar(
//        const std::string& name,
//        const std::function<bool(char)>& validator)
//{
//    auto r = stmf::NewTVar<A, PSL<TVar<A>>>::toAny(
//                val,
//                name,
//                [](const TVar<A>& tvar)
//                    { return pure<TVar<A>>(tvar); }
//                );
//    return wrap(r);
//}

/// ParserL evaluation

template <typename A>
ParseResult<A> parse(
        const ParserL<A>& psl,
        const std::string& s)
{
    if (s.empty())
        return { ParseError { "Source string is empty." }};

    ParserRuntime runtime(s, 0);
    return runParserL<A>(runtime, psl);
}

//template <typename A, typename B>
//ParserL<B> bindT(const ParserL<A>& ma,
//                 const ParserL<A>& mOnFail,
//                 const std::function<ParserL<B>(A)>& onSuccess)
//{
//    BindParserLVisitor<A, B> visitor(f);
//    std::visit(visitor, ma.psl);
//    ParserL<B> result = visitor.result;
//    return n;
//}

//template <typename T>
//ParserL<T> alt(const ParserL<T>& l, const ParserL<T>& r)
//{
//    std::function<ParserL<T>(T)> f = [](T t) { return pure<T>(t); };
//    return bindT<T, T>(l, r, f);
//}


} // namespace free
} // namespace ps

#endif // PS_FREE_PS_H
