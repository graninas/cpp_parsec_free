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
ParserL<B> bind(const ParserL<A>& ma,
             const std::function<ParserL<B>(A)>& f)
{
    ParserL<B> n;
    n.runF = [=](const std::function<S(B)>& p,
                 const std::function<S(psf::ParserF<S>)>& r)
    {
        auto fst = [=](const A& a)
        {
            ParserL<B> internal = f(a);
            return internal.runF(p, r);
        };

        return ma.runF(fst, r);
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
    n.runF = [=](const std::function<Any(A)>& p,
                 const std::function<Any(psf::ParserF<Any>)>&)
    {
        return p(a);
    };
    return n;
}

template <typename A, template <typename, typename> class Method>
ParserL<A> wrap(const Method<Any, A>& method)
{
    ParserL<A> n;

    n.runF = [=](const std::function<Any(A)>& p,
                 const std::function<Any(psf::ParserF<Any>)>& r)
    {
        psf::ParserF<A> f { method };
        psf::ParserF<Any> mapped = psf::fmap<A, Any>(p, f);
        return r(mapped);
    };

    return n;
}

template <typename A>
ParserL<Digit> parseDigit()
{
    auto r = psf::ParseDigit<A, Digit>::toAny(
                [](const Digit& d) { return d; }
                );

    return wrap(r);
}

const ParserL<Digit> digit = parseDigit<Digit>();

//template <typename A>
//ParserL<A> readTVar(const Digit& tvar)
//{
//    auto r = psf::ReadTVar<A, A>::toAny(
//                tvar,
//                [](const A& val) { return val;  });

//    return wrap(r);
//}

//template <typename A>
//ParserL<Unit> writeTVar(
//        const Digit& tvar,
//        const A& val)
//{
//    auto r = psf::WriteTVar<A, Unit>::toAny(
//                tvar,
//                val,
//                [](const Unit& unit) { return unit; }
//                );
//    return wrap(r);
//}

//template <typename A>
//ParserL<Unit> modifyTVar(
//        const Digit& tvar,
//        const std::function<A(A)>& f)
//{
//    return bind<A, Unit>(readTVar(tvar), [=](const A& val)
//    {
//        return writeTVar(tvar, f(val));
//    });
//}

///// ParserL evaluation

template <typename A>
ps::ParseResult<A> parse(
        const ParserL<A>& parserL,
        const std::string& s)
{
//    RunnerFunc<A> runner = [&](ParserRuntime& runtime)
//    {
//        return runParserL<A>(runtime, parserL);
//    };

//    return runParser<A>(context, runner);
    return runParser<A>(parserL, s);
}

//// Special version of newTVar
//template <typename A>
//Digit newTVarIO(Context& context,
//                  const A& val,
//                  const std::string& name = "")
//{
//    return atomically(context, newDigit(val, name));
//}

//// Special version of readTVar. Can be possibly optimized to not to wait for conflicts.
//template <typename A>
//A readTVarIO(Context& context,
//             const Digit& tvar)
//{
//    return atomically(context, readDigit(tvar));
//}

///// Combinators

//// Monadic values and lambdas (type inference can be better with them).

//const ParserL<Unit> mRetry = retry<Unit>();

//const ParserL<Unit> mUnit = pure<Unit>(unit);

//const auto mPure = [](const auto& val)
//{
//    return pure(val);
//};

//const auto mNewTVar = [](const auto& val)
//{
//    return newTVar(val);
//};

//const auto mReadTVar = [](const auto& tvar)
//{
//    return readTVar(tvar);
//};

//const auto mWriteTVarT = [](const auto& tvar)
//{
//    return [=](const auto& val)
//    {
//        return writeTVar(tvar, val);
//    };
//};

//const auto mWriteTVarV = [](const auto& val)
//{
//    return [=](const auto& tvar)
//    {
//        return writeTVar(tvar, val);
//    };
//};

//// Generic combinators

//// TODO: generic with using varargs and variadic templates

//// Alias for `bind`.
//template <typename A, typename B>
//ParserL<B> with(const ParserL<A>& ma,
//             const std::function<ParserL<B>(A)>& f)
//{
//    return bind<A, B>(ma, f);
//}

//template <typename A, typename B>
//ParserL<B> with(const ParserL<A>& ma,
//             const std::function<B(A)>& f)
//{
//    return bind<A, B>(ma, [=](const A& a)
//    {
//        return pure(f(a));
//    });
//}

//template <typename A, typename B, typename C>
//ParserL<C> both(const ParserL<A>& ma,
//             const ParserL<B>& mb,
//             const std::function<ParserL<C>(A, B)>& f)
//{
//    return bind<A, C>(ma, [=](const A& a){
//        return bind<B, C>(mb, [=](const B& b){
//            return f(a, b);
//        });
//    });
//}

//template <typename A, typename B, typename C>
//ParserL<C> both(const ParserL<A>& ma,
//             const ParserL<B>& mb,
//             const std::function<C(A, B)>& f)
//{
//    return both<A, B, C>(ma, mb, [=](const A& a, const B& b) { return pure(f(a, b)); });
//}

//template <typename A, typename B>
//ParserL<Unit> bothVoided(const ParserL<A>& ma,
//                      const ParserL<B>& mb)
//{
//    return both<A, B, Unit>(ma, mb, [](const A&, const B&)
//    {
//        return unit;
//    });
//}

//// TODO: rename it (`andThen`?)
//// TODO: make sequence as in Haskell
//template <typename A, typename B>
//ParserL<B> sequence(
//        const ParserL<A>& ma,
//        const ParserL<B>& mb)
//{
//    return both<A, B, B>(ma, mb, [](const A&, const B& b)
//    {
//        return b;
//    });
//}

//template <typename A>
//ParserL<Unit> voided(const ParserL<A>& ma)
//{
//    return sequence<A, Unit>(ma, pure<Unit>(unit));
//}

//template <typename A, typename B>
//ParserL<B> ifThenElse(const ParserL<A>& ma,
//                   const ParserL<B>& mOnTrue,
//                   const ParserL<B>& mOnFalse,
//                   const std::function<bool(A)>& condF)
//{
//    return bind<A, B>(ma, [=](const A& a) {
//        return condF(a) ? mOnTrue : mOnFalse;
//    });
//}

//template <typename B>
//ParserL<B> ifThenElse(const ParserL<bool>& mCond,
//                   const ParserL<B>& mOnTrue,
//                   const ParserL<B>& mOnFalse)
//{
//    return ifThenElse<bool, B>(mCond, mOnTrue, mOnFalse, id);
//}

//// Use `when` and `unless` combinators with care. Prefer ifThenElse instead.
//// Reason: it's possible to evaluate some internal transaction several times by a mistake.
//template <typename A>
//ParserL<Unit> when(const ParserL<bool>& mCond,
//                const ParserL<A>& ma)
//{
//    return ifThenElse<Unit>(mCond,
//                            voided<A>(ma),
//                            pure<Unit>(unit));
//}

//template <typename A>
//ParserL<Unit> unless(const ParserL<bool>& mCond,
//                  const ParserL<A>& ma)
//{
//    return ifThenElse<Unit>(mCond,
//                            pure<Unit>(unit),
//                            voided<A>(ma));
//}

//// Additional TVar combinators

//template <typename A, typename B>
//ParserL<B> withTVar(const ParserL<Digit>& ma,
//                 const std::function<ParserL<B>(A)>& f)
//{
//    return bind<A, B>(bind<Digit, A>(ma, mReadTVar), f);
//}

//template <typename A, typename B>
//ParserL<B> withTVar(const Digit& tvar,
//                 const std::function<ParserL<B>(A)>& f)
//{
//    return bind<A, B>(readDigit(tvar), f);
//}

//template <typename A, typename B>
//ParserL<B> withTVar(const Digit& tvar,
//                 const std::function<B(A)>& f)
//{
//    return bind<A, B>(readTVar(tvar),
//                      [=](const A& a) { return pure(f(a)); });
//}

//template <typename A, typename B>
//ParserL<Unit> whenTVar(const Digit& tvar,
//                    const std::function<bool(A)>& tvarCond,
//                    const ParserL<B>& mb)
//{
//    return withTVar<A, Unit>(tvar, [=](const A& a)
//    {
//        return when<B>(pure(tvarCond(a)), mb);
//    });
//}

//// TODO: replace by var args.

//template <typename A, typename B, typename C>
//ParserL<C> withTVars(const ParserL<Digit>& ma,
//                  const ParserL<TVar<B>>& mb,
//                  const std::function<ParserL<C>(A, B)>& f)
//{
//    return both<A, B, C>(bind<Digit, A>(ma, mReadTVar),
//                         bind<TVar<B>, B>(mb, mReadTVar),
//                         f);
//}

//template <typename A, typename B, typename C>
//ParserL<C> withTVars(const ParserL<Digit>& ma,
//                  const ParserL<TVar<B>>& mb,
//                  const std::function<C(A, B)>& f)
//{
//    return both<A, B, C>(bind<Digit, A>(ma, mReadTVar),
//                         bind<TVar<B>, B>(mb, mReadTVar),
//                         f);
//}

//template <typename A, typename B, typename C>
//ParserL<C> withTVars(const Digit& tvar1,
//                  const TVar<B>& tvar2,
//                  const std::function<C(A, B)>& f)
//{
//    return both<A, B, C>(readTVar(tvar1),
//                         readTVar(tvar2),
//                         f);
//}

//template <typename A, typename B, typename C>
//ParserL<C> withTVars(const Digit& tvar1,
//                  const TVar<B>& tvar2,
//                  const std::function<ParserL<C>(A, B)>& f)
//{
//    return both<A, B, C>(readTVar(tvar1),
//                         readTVar(tvar2),
//                         f);
//}

//template <typename A>
//ParserL<Unit> modifyTVarCurried(const Digit& tvar)
//{
//    return [=](const auto& f)
//    {
//        return modifyDigit(tvar, f);
//    };
//}

//template <typename A>
//ParserL<A> modifyTVarRet(const Digit& tvar,
//                      const std::function<A(A)>& f)
//{
//    return sequence<Unit, A>(modifyDigit(tvar, f), readDigit(tvar));
//}

//template <typename A>
//ParserL<A> writeTVarRet(const Digit& tvar,
//                     const A& a)
//{
//    return sequence<Unit, A>(writeTVar(tvar, a), readTVar(tvar));
//}

//template <typename A, typename B>
//ParserL<B> withOptional(const ParserL<std::optional<A>>& opt,
//                     const ParserL<B>& onNullOpt,
//                     const std::function<ParserL<B>(A)>& f)
//{
//    return bind<std::optional<A>, B>(opt, [=](const std::optional<A>& optA)
//    {
//        return optA.has_value() ? f(optA.value()) : onNullOpt;
//    });
//}

//template <typename A, typename B>
//ParserL<std::optional<B>> tryTVar(const Digit& tvar,
//                               const std::function<bool(A)>& tvarCond,
//                               const ParserL<std::optional<B>>& mb)
//{
//    return withTVar<A, std::optional<B>>(tvar, [=](const A& a)
//    {
//        return tvarCond(a)
//                ? mb
//                : pure<std::optional<B>>(std::nullopt);
//    });
//}

//template <typename A, typename B>
//ParserL<std::optional<B>> bindOptional(
//        const ParserL<std::optional<A>>& opt,
//        const std::function<ParserL<std::optional<B>>(A)>& f)
//{
//    return withOptional<A, std::optional<B>>(opt, pure<std::optional<B>>(std::nullopt), f);
//}

//template <typename A>
//ParserL<std::optional<A>> tryModifyTVar(
//        const Digit& tvar,
//        const std::function<std::optional<A>(A)>& f)
//{
//    return withTVar<A, std::optional<A>>(tvar, [=](const A& a)
//    {
//        std::optional<A> optNewA = f(a);
//        return optNewA.has_value()
//                ? sequence<Unit, std::optional<A>>(writeDigit(tvar, optNewA.value()),
//                                                   pure<std::optional<A>>(optNewA))
//                : pure<std::optional<A>>(optNewA);
//    });
//}

} // namespace church
} // namespace ps

#endif // PS_CHURCH_PS_H
