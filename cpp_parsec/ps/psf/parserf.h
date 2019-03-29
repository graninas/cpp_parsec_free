#ifndef PS_PSF_PSF_H
#define PS_PSF_PSF_H

#include "../types.h"

#ifdef PS_DEBUG
#include <iostream>
#endif

namespace ps
{
namespace psf
{

// PS methods

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

    ~ParseChar()
    {
#ifdef PS_DEBUG
        std::cout << "ParseChar: destructor, name: " << name << std::endl;
#endif
    }

    ParseChar()
    {
#ifdef PS_DEBUG
        std::cout << "ParseChar: empty constructor " << std::endl;
#endif
    }

    explicit ParseChar(const S& ch,
                     const std::function<Next(S)>& next)
        : ch(ch)
        , next(next)
    {
#ifdef PS_DEBUG
        std::cout << "ParseChar: constructor " << std::endl;
#endif
    }

    ParseChar(const ParseChar<Next>& other)
        : ch(other.ch)
        , next(other.next)
    {
#ifdef PS_DEBUG
        std::cout << "ParseChar: copy constructor" << std::endl;
#endif
    }

    ParseChar(const ParseChar<Next>&& other)
        : ch(other.ch)
        , next(other.next)
    {
#ifdef PS_DEBUG
        std::cout << "ParseChar: move constructor" << std::endl;
#endif
    }

    ParseChar<Next>& operator=(ParseChar<Next> other)
    {

#ifdef PS_DEBUG
        std::cout << "NewTVar: copy assignment operator" << std::endl;
#endif
        std::swap(ch, other.ch);
        std::swap(next, other.next);
        return *this;
    }

    ParseChar<Next>& operator=(ParseChar<Next>&& other)
    {
#ifdef PS_DEBUG
        std::cout << "ParseChar: move assignment operator " << std::endl;
#endif
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
        ParseChar<Ret>
    > psf;
};

} // namespace psf
} // namespace ps

#endif // PS_PSF_PSF_H

