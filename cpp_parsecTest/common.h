#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <map>
#include <list>
#include <string>
#include <tuple>

#include <functional>
#include <algorithm>

#include "ps/ps.h"

template <typename A>
void printError(const ps::ParserResult<A> &pr)
{
  if (isLeft(pr))
  {
    auto err = ps::getParseFailed(pr);
    std::cout << "\nError:\n"
              << err.message << "\n";
  }
}

#endif // COMMON_H
