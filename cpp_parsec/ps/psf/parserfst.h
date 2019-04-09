#ifndef PS_PSF_PSFST_H
#define PS_PSF_PSFST_H

#include "../types.h"

namespace ps
{
namespace psfst
{

template <typename A,
          template <typename> class P,
          typename Next>
struct TryP
{
    P<A> parser;
    std::function<Next(ParseResult<A>)> next;

    static TryP<Any, P, Next> toAny(
            const P<A>& p,
            const std::function<P<Any>(P<A>)>& pToAny,
            const std::function<Next(ParseResult<A>)>& next)
    {
        std::function<Next(ParseResult<A>)> nextCopy = next;

        std::function<A(Any)> fromAny = [](const Any& any)
        {
            return std::any_cast<A>(any);
        };

        TryP<Any, P, Next> m;
        m.parser = pToAny(p);  // cast to any
        m.next = [=](const ParseResult<Any>& resultAny)
        {
            ParseResult<A> result =
                    fmapPR<Any, A>(fromAny, resultAny); // cast from any
            return nextCopy(result);
        };
        return m;
    }
};

// Any

template <template <typename> class P, typename Next>
using TryPA = TryP<Any, P, Next>;

// Algebra

template <template <typename> class P, class Ret>
struct ParserFST
{
    std::variant<
        TryPA<P, Ret>
    > psfst;
};

} // namespace psfst
} // namespace ps

#endif // PS_PSF_PSFST_H

