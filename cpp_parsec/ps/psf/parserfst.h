#ifndef PS_PSF_PSFST_H
#define PS_PSF_PSFST_H

#include "../types.h"

namespace ps
{
namespace psfst
{

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

template <class Ret>
struct ParserFST
{
    std::variant<
        PutSt<Ret>,
        GetSt<Ret>
    > psfst;
};

} // namespace psfst
} // namespace ps

#endif // PS_PSF_PSFST_H

