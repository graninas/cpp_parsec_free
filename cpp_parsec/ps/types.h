#ifndef PS_TYPES_H
#define PS_TYPES_H

#include <functional>
#include <string>
#include <variant>
#include <list>
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
using Many = std::list<T>;

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

struct ParserFailed
{
    std::string message;
    Pos position;
};

template <typename T>
struct ParserSucceeded
{
    T parsed;
    Pos from;
    Pos to;
};

template <typename T>
using ParserResult = Either<ParserFailed, ParserSucceeded<T>>;

// unsafe get parsed
template <typename T>
ParserSucceeded<T> getParseSucceeded(const ps::ParserResult<T>& r)
{
    return std::get<ps::ParserSucceeded<T>>(r);
}

// unsafe convert success
template <typename A, typename B>
ParserSucceeded<B> convertParseSucceeded(const ps::ParserResult<A>& r)
{
    auto s = getParseSucceeded(r);
    return ParserSucceeded<B> { static_cast<B>(s.parsed), s.from, s.to };
}

// unsafe get error
template <typename T>
ParserFailed getParseFailed(const ps::ParserResult<T>& r)
{
    return std::get<ParserFailed>(r);
}

// fmap

template <typename A, typename B>
ParserResult<B> fmapPR(
        const std::function<B(A)>& f,
        const ParserResult<A>& pr)
{
    if (isLeft(pr))
    {
      return getParseFailed(pr);
    }

    auto s = getParseSucceeded(pr);
    return ParserSucceeded<B> { f(s.parsed), s.from, s.to };
}

// state

struct State
{
    Pos pos;
};

} // namespace ps

#endif // PS_TYPES_H
