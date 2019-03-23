#ifndef PS_CHURCH_PS_H
#define PS_CHURCH_PS_H

#include "../types.h"
#include "../context.h"
#include "../impl/runtime.h"
#include "psl.h"
#include "interpreter.h"

namespace ps
{
namespace church
{

template <typename A, typename B>
PSL<B> bind(const PSL<A>& ma,
             const std::function<PSL<B>(A)>& f)
{
    PSL<B> n;
    n.runF = [=](const std::function<Any(B)>& p,
                 const std::function<Any(psf::PSF<Any>)>& r)
    {
        auto fst = [=](const A& a)
        {
            PSL<B> internal = f(a);
            return internal.runF(p, r);
        };

        return ma.runF(fst, r);
    };
    return n;
}

template <typename A>
PSL<A> join(const PSL<PSL<A>>& mma)
{
    return bind<PSL<A>, A>(mma, [](const PSL<A>& ma) { return ma; });
}

template <typename A>
PSL<A> pure(const A& a)
{
    PSL<A> n;
    n.runF = [=](const std::function<Any(A)>& p,
                 const std::function<Any(psf::PSF<Any>)>&)
    {
        return p(a);
    };
    return n;
}

template <typename A, template <typename, typename> class Method>
PSL<A> wrap(const Method<Any, A>& method)
{
    PSL<A> n;

    n.runF = [=](const std::function<Any(A)>& p,
                 const std::function<Any(psf::PSF<Any>)>& r)
    {
        psf::PSF<A> f { method };
        psf::PSF<Any> mapped = psf::fmap<A, Any>(p, f);
        return r(mapped);
    };

    return n;
}

template <typename A>
PSL<A> retry()
{
    return wrap(psf::RetryA<A> {});
}

template <typename A>
PSL<TVar<A>> newTVar(
        const A& val,
        const std::string& name = "")
{
    auto r = psf::NewTVar<A, TVar<A>>::toAny(
                val,
                name,
                [](const TVar<A>& tvar) { return tvar; }
                );

    return wrap(r);
}

template <typename A>
PSL<A> readTVar(const TVar<A>& tvar)
{
    auto r = psf::ReadTVar<A, A>::toAny(
                tvar,
                [](const A& val) { return val;  });

    return wrap(r);
}

template <typename A>
PSL<Unit> writeTVar(
        const TVar<A>& tvar,
        const A& val)
{
    auto r = psf::WriteTVar<A, Unit>::toAny(
                tvar,
                val,
                [](const Unit& unit) { return unit; }
                );
    return wrap(r);
}

template <typename A>
PSL<Unit> modifyTVar(
        const TVar<A>& tvar,
        const std::function<A(A)>& f)
{
    return bind<A, Unit>(readTVar(tvar), [=](const A& val)
    {
        return writeTVar(tvar, f(val));
    });
}

/// PSL evaluation

template <typename A>
A atomically(Context& context,
             const PSL<A>& psl)
{
    RunnerFunc<A> runner = [&](AtomicRuntime& runtime)
    {
        return runPSL<A>(runtime, psl);
    };

    return runPS<A>(context, runner);
}

// Special version of newTVar
template <typename A>
TVar<A> newTVarIO(Context& context,
                  const A& val,
                  const std::string& name = "")
{
    return atomically(context, newTVar<A>(val, name));
}

// Special version of readTVar. Can be possibly optimized to not to wait for conflicts.
template <typename A>
A readTVarIO(Context& context,
             const TVar<A>& tvar)
{
    return atomically(context, readTVar<A>(tvar));
}

/// Combinators

// Monadic values and lambdas (type inference can be better with them).

const PSL<Unit> mRetry = retry<Unit>();

const PSL<Unit> mUnit = pure<Unit>(unit);

const auto mPure = [](const auto& val)
{
    return pure(val);
};

const auto mNewTVar = [](const auto& val)
{
    return newTVar(val);
};

const auto mReadTVar = [](const auto& tvar)
{
    return readTVar(tvar);
};

const auto mWriteTVarT = [](const auto& tvar)
{
    return [=](const auto& val)
    {
        return writeTVar(tvar, val);
    };
};

const auto mWriteTVarV = [](const auto& val)
{
    return [=](const auto& tvar)
    {
        return writeTVar(tvar, val);
    };
};

// Generic combinators

// TODO: generic with using varargs and variadic templates

// Alias for `bind`.
template <typename A, typename B>
PSL<B> with(const PSL<A>& ma,
             const std::function<PSL<B>(A)>& f)
{
    return bind<A, B>(ma, f);
}

template <typename A, typename B>
PSL<B> with(const PSL<A>& ma,
             const std::function<B(A)>& f)
{
    return bind<A, B>(ma, [=](const A& a)
    {
        return pure(f(a));
    });
}

template <typename A, typename B, typename C>
PSL<C> both(const PSL<A>& ma,
             const PSL<B>& mb,
             const std::function<PSL<C>(A, B)>& f)
{
    return bind<A, C>(ma, [=](const A& a){
        return bind<B, C>(mb, [=](const B& b){
            return f(a, b);
        });
    });
}

template <typename A, typename B, typename C>
PSL<C> both(const PSL<A>& ma,
             const PSL<B>& mb,
             const std::function<C(A, B)>& f)
{
    return both<A, B, C>(ma, mb, [=](const A& a, const B& b) { return pure(f(a, b)); });
}

template <typename A, typename B>
PSL<Unit> bothVoided(const PSL<A>& ma,
                      const PSL<B>& mb)
{
    return both<A, B, Unit>(ma, mb, [](const A&, const B&)
    {
        return unit;
    });
}

// TODO: rename it (`andThen`?)
// TODO: make sequence as in Haskell
template <typename A, typename B>
PSL<B> sequence(
        const PSL<A>& ma,
        const PSL<B>& mb)
{
    return both<A, B, B>(ma, mb, [](const A&, const B& b)
    {
        return b;
    });
}

template <typename A>
PSL<Unit> voided(const PSL<A>& ma)
{
    return sequence<A, Unit>(ma, pure<Unit>(unit));
}

template <typename A, typename B>
PSL<B> ifThenElse(const PSL<A>& ma,
                   const PSL<B>& mOnTrue,
                   const PSL<B>& mOnFalse,
                   const std::function<bool(A)>& condF)
{
    return bind<A, B>(ma, [=](const A& a) {
        return condF(a) ? mOnTrue : mOnFalse;
    });
}

template <typename B>
PSL<B> ifThenElse(const PSL<bool>& mCond,
                   const PSL<B>& mOnTrue,
                   const PSL<B>& mOnFalse)
{
    return ifThenElse<bool, B>(mCond, mOnTrue, mOnFalse, id);
}

// Use `when` and `unless` combinators with care. Prefer ifThenElse instead.
// Reason: it's possible to evaluate some internal transaction several times by a mistake.
template <typename A>
PSL<Unit> when(const PSL<bool>& mCond,
                const PSL<A>& ma)
{
    return ifThenElse<Unit>(mCond,
                            voided<A>(ma),
                            pure<Unit>(unit));
}

template <typename A>
PSL<Unit> unless(const PSL<bool>& mCond,
                  const PSL<A>& ma)
{
    return ifThenElse<Unit>(mCond,
                            pure<Unit>(unit),
                            voided<A>(ma));
}

// Additional TVar combinators

template <typename A, typename B>
PSL<B> withTVar(const PSL<TVar<A>>& ma,
                 const std::function<PSL<B>(A)>& f)
{
    return bind<A, B>(bind<TVar<A>, A>(ma, mReadTVar), f);
}

template <typename A, typename B>
PSL<B> withTVar(const TVar<A>& tvar,
                 const std::function<PSL<B>(A)>& f)
{
    return bind<A, B>(readTVar<A>(tvar), f);
}

template <typename A, typename B>
PSL<B> withTVar(const TVar<A>& tvar,
                 const std::function<B(A)>& f)
{
    return bind<A, B>(readTVar(tvar),
                      [=](const A& a) { return pure(f(a)); });
}

template <typename A, typename B>
PSL<Unit> whenTVar(const TVar<A>& tvar,
                    const std::function<bool(A)>& tvarCond,
                    const PSL<B>& mb)
{
    return withTVar<A, Unit>(tvar, [=](const A& a)
    {
        return when<B>(pure(tvarCond(a)), mb);
    });
}

// TODO: replace by var args.

template <typename A, typename B, typename C>
PSL<C> withTVars(const PSL<TVar<A>>& ma,
                  const PSL<TVar<B>>& mb,
                  const std::function<PSL<C>(A, B)>& f)
{
    return both<A, B, C>(bind<TVar<A>, A>(ma, mReadTVar),
                         bind<TVar<B>, B>(mb, mReadTVar),
                         f);
}

template <typename A, typename B, typename C>
PSL<C> withTVars(const PSL<TVar<A>>& ma,
                  const PSL<TVar<B>>& mb,
                  const std::function<C(A, B)>& f)
{
    return both<A, B, C>(bind<TVar<A>, A>(ma, mReadTVar),
                         bind<TVar<B>, B>(mb, mReadTVar),
                         f);
}

template <typename A, typename B, typename C>
PSL<C> withTVars(const TVar<A>& tvar1,
                  const TVar<B>& tvar2,
                  const std::function<C(A, B)>& f)
{
    return both<A, B, C>(readTVar(tvar1),
                         readTVar(tvar2),
                         f);
}

template <typename A, typename B, typename C>
PSL<C> withTVars(const TVar<A>& tvar1,
                  const TVar<B>& tvar2,
                  const std::function<PSL<C>(A, B)>& f)
{
    return both<A, B, C>(readTVar(tvar1),
                         readTVar(tvar2),
                         f);
}

template <typename A>
PSL<Unit> modifyTVarCurried(const TVar<A>& tvar)
{
    return [=](const auto& f)
    {
        return modifyTVar<A>(tvar, f);
    };
}

template <typename A>
PSL<A> modifyTVarRet(const TVar<A>& tvar,
                      const std::function<A(A)>& f)
{
    return sequence<Unit, A>(modifyTVar<A>(tvar, f), readTVar<A>(tvar));
}

template <typename A>
PSL<A> writeTVarRet(const TVar<A>& tvar,
                     const A& a)
{
    return sequence<Unit, A>(writeTVar(tvar, a), readTVar(tvar));
}

template <typename A, typename B>
PSL<B> withOptional(const PSL<std::optional<A>>& opt,
                     const PSL<B>& onNullOpt,
                     const std::function<PSL<B>(A)>& f)
{
    return bind<std::optional<A>, B>(opt, [=](const std::optional<A>& optA)
    {
        return optA.has_value() ? f(optA.value()) : onNullOpt;
    });
}

template <typename A, typename B>
PSL<std::optional<B>> tryTVar(const TVar<A>& tvar,
                               const std::function<bool(A)>& tvarCond,
                               const PSL<std::optional<B>>& mb)
{
    return withTVar<A, std::optional<B>>(tvar, [=](const A& a)
    {
        return tvarCond(a)
                ? mb
                : pure<std::optional<B>>(std::nullopt);
    });
}

template <typename A, typename B>
PSL<std::optional<B>> bindOptional(
        const PSL<std::optional<A>>& opt,
        const std::function<PSL<std::optional<B>>(A)>& f)
{
    return withOptional<A, std::optional<B>>(opt, pure<std::optional<B>>(std::nullopt), f);
}

template <typename A>
PSL<std::optional<A>> tryModifyTVar(
        const TVar<A>& tvar,
        const std::function<std::optional<A>(A)>& f)
{
    return withTVar<A, std::optional<A>>(tvar, [=](const A& a)
    {
        std::optional<A> optNewA = f(a);
        return optNewA.has_value()
                ? sequence<Unit, std::optional<A>>(writeTVar<A>(tvar, optNewA.value()),
                                                   pure<std::optional<A>>(optNewA))
                : pure<std::optional<A>>(optNewA);
    });
}

} // namespace church
} // namespace ps

#endif // PS_CHURCH_PS_H
