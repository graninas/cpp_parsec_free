#ifndef PS_TYPES_H
#define PS_TYPES_H

#include <functional>
#include <string>
#include <variant>
#include <any>

namespace ps
{

// Short definitions
using Any = std::any;
using S = std::string;
using Digit = std::uint8_t;
using Char = char;

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

//template <typename T>
//struct PResult
//{
//    T parsed;
//    std::string rest;
//};

template <typename T>
using ParseResult = Either<ParseError, T>;


} // namespace ps

#endif // PS_TYPES_H
