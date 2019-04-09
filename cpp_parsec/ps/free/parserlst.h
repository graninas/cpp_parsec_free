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
template <template <typename> class P, typename A>
struct ParserLST;

// Free methods

template <template <typename> class P, typename Ret>
struct PureFST
{
    Ret ret;
};

template <template <typename> class P, typename Ret>
struct FreeFST
{
    psfst::ParserFST<P, ParserLST<P, Ret>> psfst;
};


template <template <typename> class P, typename A>
struct ParserLST
{
    std::variant<PureFST<P, A>, FreeFST<P, A>> pslst;
};

} // namespace free
} // namespace ps

#endif // PS_FREE_PSLST_H

