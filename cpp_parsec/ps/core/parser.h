#ifndef PS_CORE_PARSER_H
#define PS_CORE_PARSER_H

#include "types.h"
#include "runtime.h"
#include "conds.h"
#include "parser/adt.h"
#include "parser/functor.h"
#include "interpreter.h"

namespace ps
{
namespace core
{

// New design

// Interface

template <typename A>
ParserResult<A> parse(
        const ParserL<A>& pst,
        const std::string& s,
        Pos from = 0)
{
    ParserRuntime runtime (s, State{});
    return runParser(runtime, pst, from);
}

template <typename A>
ParserResult<A> parse_with_runtime(
    ParserRuntime &runtime,
    const ParserL<A> &pst,
    Pos from = 0)
{
  return runParser(runtime, pst, from);
}

// Parsers

ParserL<Char> parseSymbolCond(
        const std::string& name,
        const std::function<bool(char)>& cond)
{
  std::function<bool(char)> cond_copy = cond;

  std::function<bool(Any)> condAny = [=](const Any& any)  {
      char ch = std::any_cast<char>(any);
      return cond_copy(ch);
  };


    return make_free(ParseSymbolCond<ParserL<Char>>{
                      0,
                      name,
                      condAny,
                      [](const Any& any)  {
                          char ch = std::any_cast<char>(any);
                          {
                              return make_pure(ch);
                          }
                      }
                });
}

// ParserL<std::string> parseLit(const std::string& s)
// {
//     return make_free(ParseLit<ParserL<std::string>>{
//                       0,
//                       s,
//                       [](const std::string& resS) {
//                           return make_pure(resS, 0, resS.size());
//                       }
//                 });
// }

const ParserL<Char> digit = parseSymbolCond("digit", isDigit);
const ParserL<Char> upper = parseSymbolCond("upper", isUpper);
const ParserL<Char> lower = parseSymbolCond("lower", isLower);



template <typename A>
ParserL<Many<A>> many(ParserL<A>* p)
{
  auto pCopy = std::make_shared<ParserL<Any>>(
      fmap<A, Any>([](const A &a)
                   { return a; }, *p));

  return make_free(ParseMany<ParserL<Many<A>>>{
      0,
      pCopy,
      [](const std::list<Any> &resList)
      {
        Many<A> res;
        for (const Any &any : resList)
        {
          A a = std::any_cast<A>(any);
          res.push_back(a);
        }
        return make_pure(res);
      }});
}


// Old design

// template <typename A>
// using PL = ParserL<A>;

// template <typename A, typename B>
// ParserL<B> bindPL(const ParserL<A>& ma,
//                   const std::function<ParserL<B>(A)>& f)
// {
//     return runBind<A, B>(ma, f);
// }

// template <typename A>
// ParserL<A> purePL(const A& a)
// {
//     return runPurePL(a);
// }

// template <typename A, typename B>
// ParserL<B> bind(const ParserL<A>& ma,
//                 const std::function<ParserL<B>(A)>& f)
// {
//     return runBind<A, B>(ma, f);
// }

// template <typename A, typename B>
// ParserT<B> bind(
//         const ParserT<A>& ma,
//         const std::function<ParserT<B>(A)>& f)
// {
//     // TODO
//     return runBindST<A, B>(ma, f);
// }

// // Special hacky function, do not use it.
// template <typename A, typename B>
// ParserT<B> bindSafe(
//         const ParserT<ParserResult<A>>& ma,
//         const ParserT<ParserResult<A>>& mOnFail,
//         const std::function<ParserT<B>(A)>& f)
// {
//     std::function<ParserT<B>(ParserResult<A>)> f3 =
//             [=](const ParserResult<A>& res)
//     {
//         if (isLeft(res))
//         {
//           throw std::runtime_error(getParseFailed(res).message);
//         }

//         // TODO: for new design
//         return f(getParseSucceeded(res).parsed);
//     };

//     std::function<ParserT<B>(ParserResult<A>)> f2 =
//             [=](const ParserResult<A>& res)
//     {
//         if (isLeft(res))
//         {
//             return runBindST(mOnFail, f3);
//         }

//         // TODO: for new design
//         return f(getParseSucceeded(res).parsed);
//     };

//     return runBindST(ma, f2);
// }


// template <typename A,
//           template <typename> class Method>
// static ParserL<A> wrap(const Method<ParserL<A>>& method)
// {
//     return { FreeF<A> { psf::ParserF<ParserL<A>> { method } } };
// }

// template <typename A>
// ParserT<A> pure(const A& a)
// {
//     return { PureFST<A>{ a } };
// }

// // TODO: rewrite
// template <typename A>
// ParserT<A> evalP(const PL<A>& parser)
// {
//     std::function<PL<Any>(PL<A>)> pToAny = [](const PL<A>& psl)
//     {
//         return fmap<A, Any>([](const A& a) { return a; }, psl);
//     };

//     psfst::EvalPA<ParserT<ParserResult<A>>> r
//         = psfst::EvalP<A, ParserT<ParserResult<A>>>::toAny(
//             parser,
//             pToAny,
//             [](const ParserResult<A>& pr)
//                 { return pure<ParserResult<A>>(pr); }
//             );

//     std::function<ParserLST<A>(ParserLST<ParserResult<A>>)> df
//             = [](const ParserLST<ParserResult<A>>& pslst)
//     {
//         ParserLST<A> lstMapped = bind<ParserResult<A>, A>(pslst,
//                                                                [](const ParserResult<A>& pr)
//         {
//             if (isLeft(pr))
//             {
//               throw std::runtime_error(getParseFailed(pr).message);
//             }
//             return pure<A>(getParseSucceeded(pr).parsed);
//         });

//         return lstMapped;
//     };
//     psfst::ParserFST<ParserT<ParserResult<A>>> r2 = { r };
//     auto rMapped = psfst::fmap<ParserLST<ParserResult<A>>, ParserLST<A>>(df, r2);
//     return ParserT<A> { FreeFST<A> { rMapped } };
// }

// template <typename A>
// ParserL<A> evalOrThrow(const ParserL<ParserResult<A>>& parser)
// {
//     auto f = [](const ParserResult<A>& pr)
//     {
//         if (isLeft(pr))
//         {
//           throw std::runtime_error(getParseFailed(pr).message);
//         }
//         // TODO: for new design
//         return purePL<A>(getParseSucceeded(pr).parsed);
//     };

//     return bind<ParserResult<A>, A>(parser, f);
// }

// template <typename A>
// ParserT<ParserResult<A>> tryP(const PL<A>& parser)
// {
//     std::function<PL<Any>(PL<A>)> pToAny = [](const PL<A>& psl)
//     {
//         return fmap<A, Any>([](const A& a) { return a; }, psl);
//     };

//     psfst::TryPA<ParserT<ParserResult<A>>> r
//         = psfst::TryP<A, ParserT<ParserResult<A>>>::toAny(
//             parser,
//             pToAny,
//             [](const ParserResult<A>& pr)
//                 { return pure<ParserResult<A>>(pr); }
//             );

//     auto r2 = psfst::ParserFST<ParserT<ParserResult<A>>> { r };
//     return { FreeFST<ParserResult<A>> { r2 } };
// }

// // N.B., this was commented out and said "this doesn't work.
// // But it turns out the tests pass.
// // After so much time, I don't really know what was wrong, and what in general
// // happens in this function.
// template <typename A>
// ParserT<ParserResult<A>> tryPT(const ParserT<A>& parser)
// {
//     ParserL<Unit> dummy = purePL(unit);
//     ParserT<ParserResult<Unit>> dummyP = tryP(dummy);
//     return bind<ParserResult<Unit>, ParserResult<A>>(dummyP, [=](const ParserResult<Unit>&)
//     {
//         return bind<A, ParserResult<A>>(parser, [](const A& a)
//         {
//             return pure<ParserResult<A>>(ParserSucceeded<A> { a });
//         });
//     });
// }

// template <typename A>
// ParserT<ParserResult<A>> safeP(const PL<A>& parser)
// {
//     std::function<PL<Any>(PL<A>)> pToAny = [](const PL<A>& psl)
//     {
//         return fmap<A, Any>([](const A& a) { return a; }, psl);
//     };

//     psfst::SafePA<ParserT<ParserResult<A>>> r
//         = psfst::SafeP<A, ParserT<ParserResult<A>>>::toAny(
//             parser,
//             pToAny,
//             [](const ParserResult<A>& pr)
//                 { return pure<ParserResult<A>>(pr); }
//             );

//     auto r2 = psfst::ParserFST<ParserT<ParserResult<A>>> { r };
//     return { FreeFST<ParserResult<A>> { r2 } };
// }

// ParserL<ParserResult<Char>> parseSymbolCond(
//         const std::string& name,
//         const std::function<bool(char)>& validator)
// {
//     return wrap(psf::ParseSymbolCond<ParserL<ParserResult<Char>>>{
//                       name,
//                       validator,
//                       [](const ParserResult<Char>& pr)
//                           {
//                               return purePL<ParserResult<Char>>(pr);
//                           }
//                 });
// }

// ParserL<ParserResult<std::string>> parseLit(const std::string& s)
// {
//     return wrap(psf::ParseLit<ParserL<ParserResult<std::string>>>{
//                       s,
//                       [](const ParserResult<std::string>& resS) {
//                           return purePL<ParserResult<std::string>>(resS);
//                       }
//                 });
// }

// template <typename A>
// const std::function<ParserT<Many<A>>(Many<A>, ParserT<ParserResult<A>>)> rec
//     = [](const Many<A>& acc, const ParserT<ParserResult<A>>& p)
// {
//     ParserT<Many<A>> pt = bind<ParserResult<A>, Many<A>>(
//                 p,
//                 [=](const ParserResult<A>& pr)
//     {
//         if (isLeft(pr))
//         {
//             return pure<Many<A>>(acc);
//         }
//         else
//         {
//             Many<A> acc2 = acc;
//             // TODO: for new design
//             acc2.push_back(getParseSucceeded(pr).parsed);
//             return rec<A>(acc2, p);
//         }
//     });
//     return pt;
// };

// template <typename A>
// ParserT<Many<A>> parseMany(const ParserT<ParserResult<A>>& p)
// {
//     return rec<A>(Many<A> {}, p);
// }

// template <typename A>
// ParserT<Many<A>> parseMany(const ParserL<A>& p)
// {
//     ParserT<ParserResult<A>> pt = safeP(p);

//     return parseMany<A>(pt);
// }

// template <typename A>
// const auto manyPL = [](const ParserL<A>& p) {
//     return parseMany<A>(p);
// };

// //template <typename A>
// //const auto many = [](const ParserT<A>& p) {
// //    return parseMany<A>(sa(p));
// //};


// // TODO: unify the parsers.

// const ParserL<ParserResult<Char>> digitPL    = parseSymbolCond("digit",    isDigit);
// const ParserL<ParserResult<Char>> lowerPL    = parseSymbolCond("lower",    isLower);
// const ParserL<ParserResult<Char>> upperPL    = parseSymbolCond("upper",    isUpper);
// const ParserL<ParserResult<Char>> letterPL   = parseSymbolCond("letter",   isAlpha);
// const ParserL<ParserResult<Char>> alphaNumPL = parseSymbolCond("alphaNum", isAlphaNum);
// const ParserL<ParserResult<Char>> spacePL    = parseSymbolCond("space",    isSpace);
// const ParserL<ParserResult<Char>> eolPL      = parseSymbolCond("ln",       isEol);
// const ParserL<ParserResult<Char>> crPL       = parseSymbolCond("cr",       isCr);

// const auto symbolPL = [](Char ch) {
//     return parseSymbolCond(std::string() + ch, chEq(ch));
// };

// const auto litPL = [](const std::string& s) {
//     return parseLit(s);
// };

// const ParserT<ParserResult<Char>> digitSafe    = evalP<ParserResult<Char>>(digitPL);
// const ParserT<ParserResult<Char>> lowerSafe    = evalP<ParserResult<Char>>(lowerPL);
// const ParserT<ParserResult<Char>> upperSafe    = evalP<ParserResult<Char>>(upperPL);
// const ParserT<ParserResult<Char>> letterSafe   = evalP<ParserResult<Char>>(letterPL);
// const ParserT<ParserResult<Char>> alphaNumSafe = evalP<ParserResult<Char>>(alphaNumPL);
// const ParserT<ParserResult<Char>> spaceSafe    = evalP<ParserResult<Char>>(spacePL);
// const ParserT<ParserResult<Char>> eolSafe      = evalP<ParserResult<Char>>(eolPL);
// const ParserT<ParserResult<Char>> crSafe       = evalP<ParserResult<Char>>(crPL);

// const auto symbolSafe = [](Char ch) {
//     return evalP<ParserResult<Char>>(symbolPL(ch));
// };

// const auto litSafe = [](const std::string& s) {
//     return evalP<ParserResult<std::string>>(litPL(s));
// };

// const ParserL<Char> digitThrowPL    = evalOrThrow<Char>(digitPL);
// const ParserL<Char> lowerThrowPL    = evalOrThrow<Char>(lowerPL);
// const ParserL<Char> upperThrowPL    = evalOrThrow<Char>(upperPL);
// const ParserL<Char> letterThrowPL   = evalOrThrow<Char>(letterPL);
// const ParserL<Char> alphaNumThrowPL = evalOrThrow<Char>(alphaNumPL);
// const ParserL<Char> spaceThrowPL    = evalOrThrow<Char>(spacePL);
// const ParserL<Char> eolThrowPL      = evalOrThrow<Char>(eolPL);
// const ParserL<Char> crThrowPL       = evalOrThrow<Char>(crPL);

// const auto symbolThrowPL = [](Char ch) {
//     return evalOrThrow<Char>(symbolPL(ch));
// };

// const auto litThrowPL = [](const std::string& s) {
//     return evalOrThrow<std::string>(litPL(s));
// };

// const ParserT<Char> digit    = evalP<Char>(digitThrowPL);
// const ParserT<Char> lower    = evalP<Char>(lowerThrowPL);
// const ParserT<Char> upper    = evalP<Char>(upperThrowPL);
// const ParserT<Char> letter   = evalP<Char>(letterThrowPL);
// const ParserT<Char> alphaNum = evalP<Char>(alphaNumThrowPL);
// const ParserT<Char> space    = evalP<Char>(spaceThrowPL);
// const ParserT<Char> eol      = evalP<Char>(eolThrowPL);
// const ParserT<Char> cr       = evalP<Char>(crThrowPL);

// const auto symbol = [](Char ch) {
//     return evalP<Char>(symbolThrowPL(ch));
// };

// const auto lit = [](const std::string& s) {
//     return evalP<std::string>(litThrowPL(s));
// };

// //const ParserT<Many<Char>> spaces = manyPL<Char>(spaceThrowPL);
// ParserT<Many<Char>> spaces()
// {
//     return manyPL<Char>(spaceThrowPL);
// }

// // // dummy

// // ParserT<std::string> parseString()
// // {
// //   return digit; // TODO
// // }

// // ParserT<int> parseInt()
// // {
// //   return digit; // TODO
// // }

// // ParserT<double> parseDouble()
// // {
// //   return digit; // TODO
// // }

// // const ParserT<std::string> strP = parseString();
// // const ParserT<int> intP = parseInt();
// // const ParserT<double> doubleP = parseDouble();

// template <typename A>
// ParserT<A> alt(const ParserL<A>& l, const ParserL<A>& r)
// {
//     ParserT<ParserResult<A>> lp = safeP(l);
//     ParserT<ParserResult<A>> rp = safeP(r);

//     std::function<ParserT<A>(A)> f = [](const A& a) { return pure(a); };
//     return bindSafe(lp, rp, f);
// }

// template <typename A>
// ParserT<A> alt(const ParserT<ParserResult<A>>& l,
//                const ParserT<ParserResult<A>>& r)
// {
//     std::function<ParserT<A>(A)> f = [](const A& a) { return pure(a); };
//     return bindSafe(l, r, f);
// }

// const auto constF = [](const auto& p){ return [=](auto) { return p; }; };

// template <typename A, typename B>
// ParserT<B> forgetFirst(const ParserT<A>& p1, const ParserT<B>& p2)
// {
//     return bind<A, B>(p1, constF(p2));
// }

// template <typename A, typename B>
// ParserT<A> forgetSecond(const ParserT<A>& p1, const ParserT<B>& p2)
// {
//     return bind<A, A>(p1, [=](const A& a)
//     {
//         return bind<B, A>(p2, constF(pure(a)));
//     });
// }

// template <typename A, typename B>
// ParserT<A> fst(const ParserT<A>& p1, const ParserT<B>& p2)
// {
//     return forgetSecond(p1, p2);
// }

// template <typename A, typename B>
// ParserT<B> snd(const ParserT<A>& p1, const ParserT<B>& p2)
// {
//     return forgetFirst(p1, p2);
// }

// // Applicative-like and sequential combinators.
// // TODO: this can be made better with variadic templates and varargs

// template <typename R, typename A1>
// using F1 = std::function<R(A1)>;

// template <typename R, typename A1, typename A2>
// using F2 = std::function<R(A1, A2)>;

// template <typename R, typename A1, typename A2, typename A3>
// using F3 = std::function<R(A1, A2, A3)>;

// template <typename R, typename A1, typename A2, typename A3, typename A4>
// using F4 = std::function<R(A1, A2, A3, A4)>;

// template <typename R, typename A1>
// ParserT<R> app(const F1<R, A1>& mk,
//                const ParserT<A1>& p1)
// {
//     return
//         bind<A1, R>(p1, [=](const A1& a1) {
//             return pure(mk(a1));
//             });
// }

// template <typename R, typename A1, typename A2>
// ParserT<R> app(const F2<R, A1, A2>& mk,
//                const ParserT<A1>& p1,
//                const ParserT<A2>& p2)
// {
//     return
//         bind<A1, R>(p1, [=](const A1& a1) { return
//         bind<A2, R>(p2, [=](const A2& a2) { return
//         pure(mk(a1, a2));
//         }); });
// }

// template <typename R, typename A1, typename A2, typename A3>
// ParserT<R> app(const F3<R, A1, A2, A3>& mk,
//                const ParserT<A1>& p1,
//                const ParserT<A2>& p2,
//                const ParserT<A3>& p3)
// {
//     return
//         bind<A1, R>(p1, [=](const A1& a1) { return
//         bind<A2, R>(p2, [=](const A2& a2) { return
//         bind<A3, R>(p3, [=](const A3& a3) { return
//         pure(mk(a1, a2, a3));
//         }); }); });
// }

// template <typename R, typename A1, typename A2, typename A3, typename A4>
// ParserT<R> app(const F4<R, A1, A2, A3, A4>& mk,
//                const ParserT<A1>& p1,
//                const ParserT<A2>& p2,
//                const ParserT<A3>& p3,
//                const ParserT<A4>& p4)
// {
//     return
//         bind<A1, R>(p1, [=](const A1& a1) { return
//         bind<A2, R>(p2, [=](const A2& a2) { return
//         bind<A3, R>(p3, [=](const A3& a3) { return
//         bind<A4, R>(p4, [=](const A4& a4) { return
//         pure(mk(a1, a2, a3, a4));
//         }); }); }); });
// }

// template <typename A1, typename A2>
// ParserT<A2> seq(const ParserT<A1>& p1, const ParserT<A2>& p2)
// {
//     return snd(p1, p2);
// }

// template <typename A1, typename A2, typename A3>
// ParserT<A3> seq(
//         const ParserT<A1>& p1,
//         const ParserT<A2>& p2,
//         const ParserT<A3>& p3)
// {
//     return snd(p1, snd(p2, p3));
// }

// template <typename A1, typename A2, typename A3, typename A4>
// ParserT<A4> seq(
//         const ParserT<A1>& p1,
//         const ParserT<A2>& p2,
//         const ParserT<A3>& p3,
//         const ParserT<A4>& p4)
// {
//     return snd(p1, snd(p2, snd(p3, p4)));
// }

// // More combinators

// template <typename A, typename B>
// ParserT<B> operator>> (const ParserT<A>& l, const ParserT<B>& r)
// {
//     return snd(l, r);
// }

// template <typename A, typename B>
// ParserT<A> operator<< (const ParserT<A>& l, const ParserT<B>& r)
// {
//     return fst(l, r);
// }

// template <typename A, typename B>
// ParserT<B> between(const ParserT<A>& bracketP,
//                    const ParserT<B>& p)
// {
//     return seq(bracketP, fst(p, bracketP));
// }

// // evalP: ParserT<A> evalP(const PL<A>& parser)
// // alt: ParserT<A> alt(const PL<A>& l, const PL<A>& r)
// // ParserT<A> alt(const ParserT<ParserResult<A>>& l,
// //                const ParserT<ParserResult<A>>& r)
// // ParserT<ParserResult<A>> tryP(const PL<A>& parser)

// // PL<A> -> PT<PR<A>>  (evalP)
// // PT<PR<A>> -> PT<A>  (alt)

// //?  PT<A> -> PL<A>
// //? PT<A> -> PT<ParserResult<A>>


// // Special hacky function, do not use it.
// // B == ParserResult<A>
// template <typename A>
// ParserT<ParserResult<A>> oneOrAnother(
//    const ParserT<ParserResult<A>>& ma1,
//    const ParserT<ParserResult<A>>& ma2)
// {
//     std::function<ParserT<ParserResult<A>>(ParserResult<A>)> f2 =
//             [=](const ParserResult<A>& res)
//     {
//         if (isLeft(res))
//         {
//             return ma2;
//         }

//         //  TODO: for new design
//         return tryPT(pure(getParseSucceeded(res).parsed));
//     };

//     return runBindST(ma1, f2);
// }


// template <typename A>
// ParserT<ParserResult<A>> oneOf(const std::vector<ParserT<A>>& ps)
// {
//     if (ps.empty())
//         throw std::runtime_error("Parser list is empty");

//     auto resultP = tryPT(ps[0]);
//     for(auto it = ps.begin()+1; it != ps.end(); it++) {
//         resultP = oneOrAnother<A>(resultP, tryPT(*it) );
//     };

//     return resultP;
// }


// /// ParserL evaluation

// template <typename A>
// ParserResult<A> parseP(
//         const ParserT<A>& pst,
//         const std::string& s)
// {
//     if (s.empty())
//         return { ParserFailed { "Source string is empty." }};

//     try
//     {
//         ParserRuntime runtime(s, State {0});
//         ParserResult<A> res = runParserT<A>(runtime, pst);
//         return res;
//     }
//     catch (const std::runtime_error& err)
//     {
//         return ParserFailed { err.what() };
//     }
// }

// template <typename A>
// ParserResult<A> parse(
//         const ParserT<ParserResult<A>>& pst,
//         const std::string& s)
// {
//     ParserResult<ParserResult<A>> res = parseP(pst, s);
//     if (isLeft(res))
//     {
//       auto pe = getParseFailed(res);
//       return ParserFailed{pe.message};
//     }
//     else
//     {
//         auto se = getParseSucceeded(res).parsed;
//         return se;
//     }
// }

// template <typename A>
// ParserResult<A> parse(
//         const ParserT<A>& pst,
//         const std::string& s)
// {
//     return parseP(pst, s);
// }




} // namespace core
} // namespace ps

#endif // PS_CORE_PARSER_H
