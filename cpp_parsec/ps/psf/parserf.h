#ifndef PS_PSF_PSF_H
#define PS_PSF_PSF_H

#include "../types.h"

namespace ps
{
namespace psf
{

// PS methods

template <typename A, typename Next>
struct ParseDigit
{
//    A val;
//    std::string name;
    std::function<Next(Digit)> next;

    static ParseDigit<Any, Next> toAny(
//            const A& val,
//            const std::string& name,
            const std::function<Next(Digit)>& next)
    {
        std::function<Next(Digit)> nextCopy = next;

        ParseDigit<Any, Next> m;
//        m.val = val;  // cast to any
//        m.name = name;
        m.next = [=](const Digit& d)
        {
//            Digit tvar;
//            tvar.id = tvarAny.id;
//            tvar.name = tvarAny.name;
            return nextCopy(d);
        };
        return m;
    }

    ~ParseDigit()
    {
    }

    ParseDigit()
    {
    }

    explicit ParseDigit(const std::function<Next(Digit)>& next)
        : next(next)
    {
    }

    ParseDigit(const ParseDigit<A, Next>& other)
        : next(other.next)
    {
    }

    ParseDigit(const ParseDigit<A, Next>&& other)
        : next(other.next)
    {
    }

    ParseDigit<A, Next>& operator=(ParseDigit<A, Next> other)
    {
        std::swap(next, other.next);
        return *this;
    }

    ParseDigit<A, Next>& operator=(ParseDigit<A, Next>&& other)
    {
        std::swap(next, other.next);
        return *this;
    }
};

template <typename Next>
using ParseDigitA = ParseDigit<Any, Next>;

// PSF algebraic data type

template <class Ret>
struct ParserF
{
    std::variant<
        ParseDigitA<Ret>
    > psf;
};

} // namespace psf
} // namespace ps

#endif // PS_PSF_PSF_H

