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

// ParserT generic monadic interface.

template <typename A, typename B>
ParserL<B> bindPL(const ParserL<A>& ma,
                  const std::function<ParserL<B>(A)>& f)
{
    return runBind<A, B>(ma, f);
}

template <typename A>
ParserL<A> purePL(const A& a)
{
    return runPurePL(a);
}

template <typename A, typename B>
ParserL<B> bind(const ParserL<A>& ma,
                const std::function<ParserL<B>(A)>& f)
{
    return runBind<A, B>(ma, f);
}

template <typename A, typename B>
ParserT<B> bind(
        const ParserT<A>& ma,
        const std::function<ParserT<B>(A)>& f)
{
    // TODO
    return runBindST<A, B>(ma, f);
}

// Special hacky function, do not use it.
template <typename A, typename B>
ParserT<B> bindSafe(
        const ParserT<ParserResult<A>>& ma,
        const ParserT<ParserResult<A>>& mOnFail,
        const std::function<ParserT<B>(A)>& f)
{
    std::function<ParserT<B>(ParserResult<A>)> f3 =
            [=](const ParserResult<A>& res)
    {
        if (isLeft(res))
        {
            throw std::runtime_error(getError(res).message);
        }

        return f(getParsed(res));
    };

    std::function<ParserT<B>(ParserResult<A>)> f2 =
            [=](const ParserResult<A>& res)
    {
        if (isLeft(res))
        {
            return runBindST(mOnFail, f3);
        }

        return f(getParsed(res));
    };

    return runBindST(ma, f2);
}

template <typename A,
          template <typename> class Method>
static ParserL<A> wrap(const Method<ParserL<A>>& method)
{
    return { FreeF<A> { psf::ParserF<ParserL<A>> { method } } };
}

template <typename A>
ParserT<A> pure(const A& a)
{
    return { PureFST<A>{ a } };
}

// TODO: rewrite
template <typename A>
ParserT<A> evalP(const PL<A>& parser)
{
    std::function<PL<Any>(PL<A>)> pToAny = [](const PL<A>& psl)
    {
        return fmap<A, Any>([](const A& a) { return a; }, psl);
    };

    psfst::EvalPA<ParserT<ParserResult<A>>> r
        = psfst::EvalP<A, ParserT<ParserResult<A>>>::toAny(
            parser,
            pToAny,
            [](const ParserResult<A>& pr)
                { return pure<ParserResult<A>>(pr); }
            );

    std::function<ParserLST<A>(ParserLST<ParserResult<A>>)> df
            = [](const ParserLST<ParserResult<A>>& pslst)
    {
        ParserLST<A> lstMapped = bind<ParserResult<A>, A>(pslst,
                                                               [](const ParserResult<A>& pr)
        {
            if (isLeft(pr))
            {
                throw std::runtime_error(getError(pr).message);
            }
            return pure<A>(getParsed(pr));
        });

        return lstMapped;
    };
    psfst::ParserFST<ParserT<ParserResult<A>>> r2 = { r };
    auto rMapped = psfst::fmap<ParserLST<ParserResult<A>>, ParserLST<A>>(df, r2);
    return ParserT<A> { FreeFST<A> { rMapped } };
}

template <typename A>
ParserL<A> evalOrThrow(const ParserL<ParserResult<A>>& parser)
{
    auto f = [](const ParserResult<A>& pr)
    {
        if (isLeft(pr))
        {
            throw std::runtime_error(getError(pr).message);
        }
        return purePL<A>(getParsed(pr));
    };

    return bind<ParserResult<A>, A>(parser, f);
}

template <typename A>
ParserT<ParserResult<A>> tryP(const PL<A>& parser)
{
    std::function<PL<Any>(PL<A>)> pToAny = [](const PL<A>& psl)
    {
        return fmap<A, Any>([](const A& a) { return a; }, psl);
    };

    psfst::TryPA<ParserT<ParserResult<A>>> r
        = psfst::TryP<A, ParserT<ParserResult<A>>>::toAny(
            parser,
            pToAny,
            [](const ParserResult<A>& pr)
                { return pure<ParserResult<A>>(pr); }
            );

    auto r2 = psfst::ParserFST<ParserT<ParserResult<A>>> { r };
    return { FreeFST<ParserResult<A>> { r2 } };
}

// This is experiment (it does not work)
//template <typename A>
//ParserT<ParserResult<A>> tryP(const ParserT<A>& parser)
//{
//    ParserL<Unit> dummy = purePL(unit);
//    ParserT<ParserResult<Unit>> dummyP = tryP(dummy);
//    return bind<ParserResult<Unit>, ParserResult<A>>(dummyP, [=](const ParserResult<Unit>&)
//    {
//        return bind<A, ParserResult<A>>(parser, [](const A& a)
//        {
//            return pure<ParserResult<A>>(ParserSucceeded<A> { a });
//        });
//    });
//}

template <typename A>
ParserT<ParserResult<A>> safeP(const PL<A>& parser)
{
    std::function<PL<Any>(PL<A>)> pToAny = [](const PL<A>& psl)
    {
        return fmap<A, Any>([](const A& a) { return a; }, psl);
    };

    psfst::SafePA<ParserT<ParserResult<A>>> r
        = psfst::SafeP<A, ParserT<ParserResult<A>>>::toAny(
            parser,
            pToAny,
            [](const ParserResult<A>& pr)
                { return pure<ParserResult<A>>(pr); }
            );

    auto r2 = psfst::ParserFST<ParserT<ParserResult<A>>> { r };
    return { FreeFST<ParserResult<A>> { r2 } };
}

ParserL<ParserResult<Char>> parseSymbolCond(
        const std::string& name,
        const std::function<bool(char)>& validator)
{
    return wrap(psf::ParseSymbolCond<ParserL<ParserResult<Char>>>{
                      name,
                      validator,
                      [](const ParserResult<Char>& pr)
                          {
                              return purePL<ParserResult<Char>>(pr);
                          }
                });
}

ParserL<ParserResult<std::string>> parseLit(const std::string& s)
{
    return wrap(psf::ParseLit<ParserL<ParserResult<std::string>>>{
                      s,
                      [](const ParserResult<std::string>& resS) {
                          return purePL<ParserResult<std::string>>(resS);
                      }
                });
}

template <typename A>
const std::function<ParserT<Many<A>>(Many<A>, ParserT<ParserResult<A>>)> rec
    = [](const Many<A>& acc, const ParserT<ParserResult<A>>& p)
{
    ParserT<Many<A>> pt = bind<ParserResult<A>, Many<A>>(
                p,
                [=](const ParserResult<A>& pr)
    {
        if (isLeft(pr))
        {
            return pure<Many<A>>(acc);
        }
        else
        {
            Many<A> acc2 = acc;
            acc2.push_back(getParsed(pr));
            return rec<A>(acc2, p);
        }
    });
    return pt;
};

template <typename A>
ParserT<Many<A>> parseMany(const ParserT<ParserResult<A>>& p)
{
    return rec<A>(Many<A> {}, p);
}

template <typename A>
ParserT<Many<A>> parseMany(const ParserL<A>& p)
{
    ParserT<ParserResult<A>> pt = safeP(p);

    return parseMany<A>(pt);
}

template <typename A>
const auto manyPL = [](const ParserL<A>& p) {
    return parseMany<A>(p);
};

//template <typename A>
//const auto many = [](const ParserT<A>& p) {
//    return parseMany<A>(sa(p));
//};

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

const auto isSpace = [](char ch)
{
    return ch == ' ';
};

const auto isEol = [](char ch)
{
    return ch == '\n';
};

const auto isCr = [](char ch)
{
    return ch == '\r';
};

// TODO: unify the parsers.

const ParserL<ParserResult<Char>> digitPL    = parseSymbolCond("digit",    isDigit);
const ParserL<ParserResult<Char>> lowerPL    = parseSymbolCond("lower",    isLower);
const ParserL<ParserResult<Char>> upperPL    = parseSymbolCond("upper",    isUpper);
const ParserL<ParserResult<Char>> letterPL   = parseSymbolCond("letter",   isAlpha);
const ParserL<ParserResult<Char>> alphaNumPL = parseSymbolCond("alphaNum", isAlphaNum);
const ParserL<ParserResult<Char>> spacePL    = parseSymbolCond("space",    isSpace);
const ParserL<ParserResult<Char>> eolPL      = parseSymbolCond("ln",       isEol);
const ParserL<ParserResult<Char>> crPL       = parseSymbolCond("cr",       isCr);

const auto symbolPL = [](Char ch) {
    return parseSymbolCond(std::string() + ch, chEq(ch));
};

const auto litPL = [](const std::string& s) {
    return parseLit(s);
};

const ParserT<ParserResult<Char>> digitSafe    = evalP<ParserResult<Char>>(digitPL);
const ParserT<ParserResult<Char>> lowerSafe    = evalP<ParserResult<Char>>(lowerPL);
const ParserT<ParserResult<Char>> upperSafe    = evalP<ParserResult<Char>>(upperPL);
const ParserT<ParserResult<Char>> letterSafe   = evalP<ParserResult<Char>>(letterPL);
const ParserT<ParserResult<Char>> alphaNumSafe = evalP<ParserResult<Char>>(alphaNumPL);
const ParserT<ParserResult<Char>> spaceSafe    = evalP<ParserResult<Char>>(spacePL);
const ParserT<ParserResult<Char>> eolSafe      = evalP<ParserResult<Char>>(eolPL);
const ParserT<ParserResult<Char>> crSafe       = evalP<ParserResult<Char>>(crPL);

const auto symbolSafe = [](Char ch) {
    return evalP<ParserResult<Char>>(symbolPL(ch));
};

const auto litSafe = [](const std::string& s) {
    return evalP<ParserResult<std::string>>(litPL(s));
};

const ParserL<Char> digitThrowPL    = evalOrThrow<Char>(digitPL);
const ParserL<Char> lowerThrowPL    = evalOrThrow<Char>(lowerPL);
const ParserL<Char> upperThrowPL    = evalOrThrow<Char>(upperPL);
const ParserL<Char> letterThrowPL   = evalOrThrow<Char>(letterPL);
const ParserL<Char> alphaNumThrowPL = evalOrThrow<Char>(alphaNumPL);
const ParserL<Char> spaceThrowPL    = evalOrThrow<Char>(spacePL);
const ParserL<Char> eolThrowPL      = evalOrThrow<Char>(eolPL);
const ParserL<Char> crThrowPL       = evalOrThrow<Char>(crPL);

const auto symbolThrowPL = [](Char ch) {
    return evalOrThrow<Char>(symbolPL(ch));
};

const auto litThrowPL = [](const std::string& s) {
    return evalOrThrow<std::string>(litPL(s));
};

const ParserT<Char> digit    = evalP<Char>(digitThrowPL);
const ParserT<Char> lower    = evalP<Char>(lowerThrowPL);
const ParserT<Char> upper    = evalP<Char>(upperThrowPL);
const ParserT<Char> letter   = evalP<Char>(letterThrowPL);
const ParserT<Char> alphaNum = evalP<Char>(alphaNumThrowPL);
const ParserT<Char> space    = evalP<Char>(spaceThrowPL);
const ParserT<Char> eol      = evalP<Char>(eolThrowPL);
const ParserT<Char> cr       = evalP<Char>(crThrowPL);

const auto symbol = [](Char ch) {
    return evalP<Char>(symbolThrowPL(ch));
};

const auto lit = [](const std::string& s) {
    return evalP<std::string>(litThrowPL(s));
};

//const ParserT<Many<Char>> spaces = manyPL<Char>(spaceThrowPL);
ParserT<Many<Char>> spaces()
{
    return manyPL<Char>(spaceThrowPL);
}

// dummy

ParserT<std::string> parseString()
{

}

ParserT<int> parseInt()
{

}

ParserT<double> parseDouble()
{

}

const ParserT<std::string> strP = parseString();
const ParserT<int> intP = parseInt();
const ParserT<double> doubleP = parseDouble();

template <typename A>
ParserT<A> alt(const ParserL<A>& l, const ParserL<A>& r)
{
    ParserT<ParserResult<A>> lp = safeP(l);
    ParserT<ParserResult<A>> rp = safeP(r);

    std::function<ParserT<A>(A)> f = [](const A& a) { return pure(a); };
    return bindSafe(lp, rp, f);
}

template <typename A>
ParserT<A> alt(const ParserT<ParserResult<A>>& l,
               const ParserT<ParserResult<A>>& r)
{
    std::function<ParserT<A>(A)> f = [](const A& a) { return pure(a); };
    return bindSafe(l, r, f);
}

const auto constF = [](const auto& p){ return [=](auto) { return p; }; };

template <typename A, typename B>
ParserT<B> forgetFirst(const ParserT<A>& p1, const ParserT<B>& p2)
{
    return bind<A, B>(p1, constF(p2));
}

template <typename A, typename B>
ParserT<A> forgetSecond(const ParserT<A>& p1, const ParserT<B>& p2)
{
    return bind<A, A>(p1, [=](const A& a)
    {
        return bind<B, A>(p2, constF(pure(a)));
    });
}

template <typename A, typename B>
ParserT<A> fst(const ParserT<A>& p1, const ParserT<B>& p2)
{
    return forgetSecond(p1, p2);
}

template <typename A, typename B>
ParserT<B> snd(const ParserT<A>& p1, const ParserT<B>& p2)
{
    return forgetFirst(p1, p2);
}

// Applicative-like and sequential combinators.
// TODO: this can be made better with variadic templates and varargs

template <typename R, typename A1>
using F1 = std::function<R(A1)>;

template <typename R, typename A1, typename A2>
using F2 = std::function<R(A1, A2)>;

template <typename R, typename A1, typename A2, typename A3>
using F3 = std::function<R(A1, A2, A3)>;

template <typename R, typename A1, typename A2, typename A3, typename A4>
using F4 = std::function<R(A1, A2, A3, A4)>;

template <typename R, typename A1>
ParserT<R> app(const F1<R, A1>& mk,
               const ParserT<A1>& p1)
{
    return
        bind<A1, R>(p1, [=](const A1& a1) {
            return pure(mk(a1));
            });
}

template <typename R, typename A1, typename A2>
ParserT<R> app(const F2<R, A1, A2>& mk,
               const ParserT<A1>& p1,
               const ParserT<A2>& p2)
{
    return
        bind<A1, R>(p1, [=](const A1& a1) { return
        bind<A2, R>(p2, [=](const A2& a2) { return
        pure(mk(a1, a2));
        }); });
}

template <typename R, typename A1, typename A2, typename A3>
ParserT<R> app(const F3<R, A1, A2, A3>& mk,
               const ParserT<A1>& p1,
               const ParserT<A2>& p2,
               const ParserT<A3>& p3)
{
    return
        bind<A1, R>(p1, [=](const A1& a1) { return
        bind<A2, R>(p2, [=](const A2& a2) { return
        bind<A3, R>(p3, [=](const A3& a3) { return
        pure(mk(a1, a2, a3));
        }); }); });
}

template <typename R, typename A1, typename A2, typename A3, typename A4>
ParserT<R> app(const F4<R, A1, A2, A3, A4>& mk,
               const ParserT<A1>& p1,
               const ParserT<A2>& p2,
               const ParserT<A3>& p3,
               const ParserT<A4>& p4)
{
    return
        bind<A1, R>(p1, [=](const A1& a1) { return
        bind<A2, R>(p2, [=](const A2& a2) { return
        bind<A3, R>(p3, [=](const A3& a3) { return
        bind<A4, R>(p4, [=](const A4& a4) { return
        pure(mk(a1, a2, a3, a4));
        }); }); }); });
}

template <typename A1, typename A2>
ParserT<A2> seq(const ParserT<A1>& p1, const ParserT<A2>& p2)
{
    return snd(p1, p2);
}

template <typename A1, typename A2, typename A3>
ParserT<A3> seq(
        const ParserT<A1>& p1,
        const ParserT<A2>& p2,
        const ParserT<A3>& p3)
{
    return snd(p1, snd(p2, p3));
}

template <typename A1, typename A2, typename A3, typename A4>
ParserT<A4> seq(
        const ParserT<A1>& p1,
        const ParserT<A2>& p2,
        const ParserT<A3>& p3,
        const ParserT<A4>& p4)
{
    return snd(p1, snd(p2, snd(p3, p4)));
}

// More combinators

template <typename A, typename B>
ParserT<B> operator>> (const ParserT<A>& l, const ParserT<B>& r)
{
    return snd(l, r);
}

template <typename A, typename B>
ParserT<A> operator<< (const ParserT<A>& l, const ParserT<B>& r)
{
    return fst(l, r);
}

template <typename A, typename B>
ParserT<B> between(const ParserT<A>& bracketP,
                   const ParserT<B>& p)
{
    return seq(bracketP, fst(p, bracketP));
}


/// ParserL evaluation

template <typename A>
ParserResult<A> parseP(
        const ParserT<A>& pst,
        const std::string& s)
{
    if (s.empty())
        return { ParserFailed { "Source string is empty." }};

    try
    {
        ParserRuntime runtime(s, State {0});
        ParserResult<A> res = runParserT<A>(runtime, pst);
        return res;
    }
    catch (const std::runtime_error& err)
    {
        return ParserFailed { err.what() };
    }
}

template <typename A>
ParserResult<A> parse(
        const ParserT<ParserResult<A>>& pst,
        const std::string& s)
{
    ParserResult<ParserResult<A>> res = parseP(pst, s);
    if (isLeft(res))
    {
        auto pe = getError(res);
        return ParserFailed { pe.message };
    }
    else
    {
        auto se = getParsed(res);
        return se;
    }
}

template <typename A>
ParserResult<A> parse(
        const ParserT<A>& pst,
        const std::string& s)
{
    return parseP(pst, s);
}

} // namespace free
} // namespace ps

#endif // PS_FREE_PS_H
