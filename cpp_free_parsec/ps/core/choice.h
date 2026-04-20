#ifndef PS_CORE_CHOICE_H
#define PS_CORE_CHOICE_H

#include "parser.h"

namespace ps {
namespace core {

// Variadic template implementation of choice
// Tries each parser in sequence and returns the result of the first one that succeeds.
template <typename Parser1, typename... Parsers>
Parser<typename Parser1::ResultType> choice(const Parser1& p1, const Parsers&... parsers) {
    if constexpr (sizeof...(parsers) == 0) {
        return p1;
    } else {
        return alt(p1, choice(parsers...));
    }
}

} // namespace core
} // namespace ps

#endif // PS_CORE_CHOICE_H
