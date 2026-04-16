#ifndef PS_CORE_ADT_METHODS_H
#define PS_CORE_ADT_METHODS_H

#include "../types.h"

namespace ps
{
namespace core
{

// PS methods

template <typename Next>
struct ParseSymbolCond
{
    Pos from;

    std::string name;
    std::function<bool(char)> validator;
    std::function<Next(char)> next;
};

template <typename Next>
struct ParseLit
{
  Pos from;

  std::string s;
  std::function<Next(std::string)> next;
};

template <typename Next>
struct GetSt
{
  Pos from;
  std::function<Next(State)> next;
};

template <typename Next>
struct PutSt
{
    Pos from;
    State st;
    std::function<Next(Unit)> next;
};


template <class Ret>
struct ParserADT        // TODO: rename to Methods
{
    std::variant<
        ParseSymbolCond<Ret>,
        ParseLit<Ret>,
        GetSt<Ret>,
        PutSt<Ret>
    > psf;
};

} // namespace core
} // namespace ps

#endif // PS_CORE_ADT_METHODS_H
