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
    std::function<Next(S)> next;

    static ParseDigit<Next> toAny(
            const std::function<Next(Digit)>& next)
    {
        std::function<Next(Digit)> nextCopy = next;
        ParseDigit<Next> m;

        // This is not needed: no toAny / fromAny conversion
        m.next = [=](const Digit& dAny)
        {
            return nextCopy(dAny);
        };
        return m;
    }

    ~ParseDigit(){}
    ParseDigit(){}

    explicit ParseDigit(const Digit& d,
                     const std::function<Next(Digit)>& next)
        : d(d)
        , next(next)
    {}

    ParseDigit(const ParseDigit<Next>& other)
        : d(other.d)
        , next(other.next)
    {
    }

    ParseDigit(const ParseDigit<Next>&& other)
        : d(other.d)
        , next(other.next)
    {
    }

    ParseDigit<Next>& operator=(ParseDigit<Next> other)
    {
        std::swap(d, other.d);
        std::swap(next, other.next);
        return *this;
    }

    ParseDigit<Next>& operator=(ParseDigit<Next>&& other)
    {
        std::swap(d, other.d);
        std::swap(next, other.next);
        return *this;
    }
};

template <typename Next>
struct ParseChar
{
    S ch;
    std::function<Next(S)> next;

    static ParseChar<Next> toAny(
            const S& ch,
            const std::function<Next(S)>& next)
    {
        std::function<Next(S)> nextCopy = next;

        ParseChar<Next> m;
        m.ch = ch;

        // This is not needed: no toAny / fromAny conversion
        m.next = [=](const S& sAny)
        {
            return nextCopy(sAny);
        };
        return m;
    }

    ~ParseChar(){}
    ParseChar(){}

    explicit ParseChar(const S& ch,
                     const std::function<Next(S)>& next)
        : ch(ch)
        , next(next)
    {}

    ParseChar(const ParseChar<Next>& other)
        : ch(other.ch)
        , next(other.next)
    {
    }

    ParseChar(const ParseChar<Next>&& other)
        : ch(other.ch)
        , next(other.next)
    {
    }

    ParseChar<Next>& operator=(ParseChar<Next> other)
    {
        std::swap(ch, other.ch);
        std::swap(next, other.next);
        return *this;
    }

    ParseChar<Next>& operator=(ParseChar<Next>&& other)
    {
        std::swap(ch, other.ch);
        std::swap(next, other.next);
        return *this;
    }
};

// PSF algebraic data type

template <class Ret>
struct ParserF
{
    std::variant<
        ParseChar<Ret>,
        ParseDigit<Ret>
    > psf;
};

} // namespace psf
} // namespace ps

#endif // PS_PSF_PSF_H

