#ifndef PS_TYPES_H
#define PS_TYPES_H

#include <functional>
#include <variant>
#include <any>

#include "tvar.h"

namespace ps
{

// Short definitions
using Any = std::any;
using TVarAny = TVar<std::any>;

struct Unit
{
};

const Unit unit = {};

// TODO: && and std::forward??
const auto id = [](const auto& val) { return val; };

} // namespace ps

#endif // PS_TYPES_H
