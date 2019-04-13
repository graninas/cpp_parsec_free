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

    std::function<Next(ParserResult<Char>)> next;
};

template <typename Next>
struct ParseLit
{
    std::string s;

    std::function<Next(ParserResult<std::string>)> next;
};

template <typename Next>
struct PutSt
{
    State st;
    std::function<Next(ParserResult<Unit>)> next;
};

template <typename Next>
struct GetSt
{
    std::function<Next(ParserResult<State>)> next;
};

// PSF algebraic data type

template <class Ret>
struct ParserF
{
    std::variant<
        ParseSymbolCond<Ret>,
        ParseLit<Ret>,
        PutSt<Ret>,
        GetSt<Ret>
    > psf;
};

} // namespace psf
} // namespace ps

#endif // PS_PSF_PSF_H

