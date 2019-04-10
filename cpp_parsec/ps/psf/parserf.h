#ifndef PS_PSF_PSF_H
#define PS_PSF_PSF_H

#include "../types.h"

namespace ps
{
namespace psf
{

// PS methods

template <typename Next>
struct ParseSymbolCond
{
    std::string name;
    std::function<bool(char)> validator;

    std::function<Next(Char)> next;
};

template <typename Next>
struct PutSt
{
    State st;
    std::function<Next(Unit)> next;
};

template <typename Next>
struct GetSt
{
    std::function<Next(State)> next;
};

// PSF algebraic data type

template <class Ret>
struct ParserF
{
    std::variant<
        ParseSymbolCond<Ret>,
        PutSt<Ret>,
        GetSt<Ret>
    > psf;
};

} // namespace psf
} // namespace ps

#endif // PS_PSF_PSF_H

