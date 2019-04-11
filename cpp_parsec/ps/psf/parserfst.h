#ifndef PS_PSF_PSFST_H
#define PS_PSF_PSFST_H

#include "../types.h"
#include "../free/parserl.h"

namespace ps
{
namespace psfst
{

template <typename A>
using PSL = ps::free::ParserL<A>;

template <typename A, typename Next>
struct SafeP
{
    ps::free::ParserL<A> parser;
    std::function<Next(ParserResult<A>)> next;

    static SafeP<Any, Next> toAny(
            const PSL<A>& p,
            const std::function<PSL<Any>(PSL<A>)>& pToAny,
            const std::function<Next(ParserResult<A>)>& next)
    {
        std::function<Next(ParserResult<A>)> nextCopy = next;

        std::function<A(Any)> fromAny = [](const Any& any)
        {
            return std::any_cast<A>(any);
        };

        SafeP<Any, Next> m;
        m.parser = pToAny(p);  // cast to any
        m.next = [=](const ParserResult<Any>& resultAny)
        {
            ParserResult<A> result =
                    fmapPR<Any, A>(fromAny, resultAny); // cast from any
            return nextCopy(result);
        };
        return m;
    }
};

template <typename A, typename Next>
struct TryP
{
    PSL<A> parser;
    std::function<Next(ParserResult<A>)> next;

    static TryP<Any, Next> toAny(
            const PSL<A>& p,
            const std::function<PSL<Any>(PSL<A>)>& pToAny,
            const std::function<Next(ParserResult<A>)>& next)
    {
        std::function<Next(ParserResult<A>)> nextCopy = next;

        std::function<A(Any)> fromAny = [](const Any& any)
        {
            return std::any_cast<A>(any);
        };

        TryP<Any, Next> m;
        m.parser = pToAny(p);  // cast to any
        m.next = [=](const ParserResult<Any>& resultAny)
        {
            ParserResult<A> result =
                    fmapPR<Any, A>(fromAny, resultAny); // cast from any
            return nextCopy(result);
        };
        return m;
    }
};

template <typename A, typename Next>
struct EvalP
{
    PSL<A> parser;
    std::function<Next(ParserResult<A>)> next;

    static EvalP<Any, Next> toAny(
            const PSL<A>& p,
            const std::function<PSL<Any>(PSL<A>)>& pToAny,
            const std::function<Next(ParserResult<A>)>& next)
    {
        std::function<Next(ParserResult<A>)> nextCopy = next;

        std::function<A(Any)> fromAny = [](const Any& any)
        {
            return std::any_cast<A>(any);
        };

        EvalP<Any, Next> m;
        m.parser = pToAny(p);  // cast to any
        m.next = [=](const ParserResult<Any>& resultAny)
        {
            ParserResult<A> result =
                    fmapPR<Any, A>(fromAny, resultAny); // cast from any
            return nextCopy(result);
        };
        return m;
    }
};

// Any

template <typename Next>
using SafePA = SafeP<Any, Next>;

template <typename Next>
using TryPA = TryP<Any, Next>;

template <typename Next>
using EvalPA = EvalP<Any, Next>;

// Algebra

template <class Ret>
struct ParserFST
{
    std::variant<
        SafePA<Ret>,
        TryPA<Ret>,
        EvalPA<Ret>
    > psfst;
};

} // namespace psfst
} // namespace ps

#endif // PS_PSF_PSFST_H

