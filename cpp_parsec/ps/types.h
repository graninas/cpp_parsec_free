#ifndef PS_TYPES_H
#define PS_TYPES_H

#include <functional>
#include <string>
#include <variant>
#include <any>
#include <vector>

namespace ps
{

// Short definitions
using Any = std::any;
using S = std::string;
using Digit = std::uint8_t;
using Char = char;
using Pos = size_t;

template <typename T>
using Many = std::vector<T>;

struct Unit
{
};

const Unit unit = {};

// TODO: && and std::forward??
const auto id = [](const auto& val) { return val; };


template <typename E, typename T>
using Either = std::variant<E, T>;

template <typename E, typename T>
bool isRight(const Either<E,T>& e)
{
    return std::holds_alternative<T>(e);
}

template <typename E, typename T>
bool isLeft(const Either<E,T>& e)
{
    return std::holds_alternative<E>(e);
}

struct ParseError
{
    std::string message;
};

template <typename T>
struct ParseSuccess
{
    T parsed;
};

template <typename T>
using ParseResult = Either<ParseError, ParseSuccess<T>>;

// unsafe get parsed
template <typename T>
T getParsed(const ps::ParseResult<T>& r)
{
    ParseSuccess<T> s = std::get<ps::ParseSuccess<T>>(r);
    return s.parsed;
}

// unsafe get error
template <typename T>
ParseError getError(const ps::ParseResult<T>& r)
{
    return std::get<ParseError>(r);
}

// fmap

template <typename A, typename B>
ParseResult<B> fmapPR(
        const std::function<B(A)>& f,
        const ParseResult<A>& pr)
{
    if (isLeft(pr))
    {
        return getError(pr);
    }

    A parsed = getParsed(pr);
    return ParseSuccess<B> { f(parsed) };
}

// state

struct State
{
    Pos pos;
};

} // namespace ps

#endif // PS_TYPES_H
