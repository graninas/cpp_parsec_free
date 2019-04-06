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
struct FailWith
{
    std::string message;

    std::function<Next(Any)> next;
};

// PSF algebraic data type

template <class Ret>
struct ParserF
{
    std::variant<
        ParseSymbolCond<Ret>,
        FailWith<Ret>
    > psf;
};

} // namespace psf
} // namespace ps

#endif // PS_PSF_PSF_H

