#ifndef PS_TYPES_H
#define PS_TYPES_H

#include <functional>
#include <variant>
#include <any>

namespace ps
{

// Short definitions
using Any = std::any;
using S = std::string;
using Digit = std::uint8_t;

struct Unit
{
};

const Unit unit = {};

// TODO: && and std::forward??
const auto id = [](const auto& val) { return val; };

} // namespace ps

#endif // PS_TYPES_H
