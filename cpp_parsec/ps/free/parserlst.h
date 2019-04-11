#ifndef PS_FREE_PSLST_H
#define PS_FREE_PSLST_H

#include "../types.h"
#include "../psf/parserfst.h"

namespace ps
{
namespace free
{

// TODO: unify

// Forward declaration
template <typename A>
struct ParserLST;

// Free methods

template <typename Ret>
struct PureFST
{
    Ret ret;
};

template <typename Ret>
struct FreeFST
{
    psfst::ParserFST<ParserLST<Ret>> psfst;
};

template <typename A>
struct ParserLST
{
    std::variant<PureFST<A>, FreeFST<A>> pslst;
};

template <typename A>
using ParserT = ParserLST<A>;

} // namespace free
} // namespace ps

#endif // PS_FREE_PSLST_H

