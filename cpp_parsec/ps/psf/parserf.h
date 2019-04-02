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

    ~ParseDigit() {}

    ParseDigit() {}

    explicit ParseDigit(const std::function<Next(Digit)>& next)
        : next(next)
    {
    }

    ParseDigit(const ParseDigit<Next>& other)
        : next(other.next)
    {
    }

    ParseDigit(const ParseDigit<Next>&& other)
        : next(other.next)
    {
    }

    ParseDigit<Next>& operator=(ParseDigit<Next> other)
    {
        std::swap(next, other.next);
        return *this;
    }

    ParseDigit<Next>& operator=(ParseDigit<Next>&& other)
    {
        std::swap(next, other.next);
        return *this;
    }
};

// PSF algebraic data type

template <class Ret>
struct ParserF
{
    std::variant<
        ParseDigit<Ret>
    > psf;
};

} // namespace psf
} // namespace ps

#endif // PS_PSF_PSF_H

