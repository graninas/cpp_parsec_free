#ifndef PS_PSF_PSF_H
#define PS_PSF_PSF_H

#include "../types.h"

namespace ps
{
namespace psf
{

// PS methods

template <typename Next>
struct ParseDigit
{
    std::function<Next(Digit)> next;
};

template <typename Next>
struct ParseUpperCaseChar
{
    std::function<Next(Char)> next;
};

template <typename Next>
struct ParseLowerCaseChar
{
    std::function<Next(Char)> next;
};

template <typename Next>
struct ParseSymbol
{
    Char symbol;
    std::function<Next(Char)> next;
};

// PSF algebraic data type

template <class Ret>
struct ParserF
{
    std::variant<
        ParseDigit<Ret>,
        ParseUpperCaseChar<Ret>,
        ParseLowerCaseChar<Ret>,
        ParseSymbol<Ret>
    > psf;
};

} // namespace psf
} // namespace ps

#endif // PS_PSF_PSF_H

