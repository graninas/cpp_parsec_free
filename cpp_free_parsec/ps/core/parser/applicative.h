#ifndef PS_CORE_PARSER_APPLICATIVE_H
#define PS_CORE_PARSER_APPLICATIVE_H

#include "../types.h"
#include "functor.h"
#include "bind.h"

namespace ps
{
namespace core
{

// Applicative combinator
// Combines two parsers: one producing a function and the other producing a value.
// Applies the function from the first parser to the value from the second parser.
template <typename A, typename B>
Parser<B> applicative(const Parser<std::function<B(A)>> &pf,
                      const Parser<A> &pa)
{
    return bind<std::function<B(A)>, B>(
        pf, [=](const std::function<B(A)> &f) {
            return fmap<A, B>(f, pa);
        });
}

} // namespace core
} // namespace ps

#endif // PS_CORE_PARSER_APPLICATIVE_H
