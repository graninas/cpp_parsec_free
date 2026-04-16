#ifndef PS_CORE_TYPES_H
#define PS_CORE_TYPES_H

#include <functional>
#include <string>
#include <variant>
#include <list>
#include <any>
#include <vector>

namespace ps
{
namespace core
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
    Pos pos;
};

template <typename T>
struct ParserSucceeded
{
    T parsed;   // TODO: Might not be needed, but for now we keep it for simplicity
    Pos from;
    Pos to;
};

template <typename T>
using ParserResult = Either<ParserFailed, ParserSucceeded<T>>;

// unsafe get parsed
template <typename T>
ParserSucceeded<T> getParseSucceeded(const ParserResult<T>& r)
{
    return std::get<ParserSucceeded<T>>(r);
}

// unsafe convert success
template <typename A, typename B>
ParserSucceeded<B> convertParseSucceeded(const ParserResult<A>& r,
                                       const std::function<B(A)>& f)
{
  ParserSucceeded<A> s = getParseSucceeded(r);
  ParserSucceeded<B> res;
  res.parsed = f(s.parsed);
  res.from = s.from;
  res.to = s.to;
  return res;
}

// unsafe get error
template <typename T>
ParserFailed getParseFailed(const ParserResult<T>& r)
{
    return std::get<ParserFailed>(r);
}

// state

// TODO: redesign

struct State
{
    Pos pos;      // Not needed
};

} // namespace core
} // namespace ps

#endif // PS_CORE_TYPES_H
