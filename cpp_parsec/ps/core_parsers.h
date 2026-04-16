#ifndef PS_CORE_PARSERS_H
#define PS_CORE_PARSERS_H

#include <string>
#include <functional>

#include "types.h"

namespace ps
{
namespace core
{

  template <typename Single>
  ParserResult<Single> parseSingle(
      ParserRuntime &runtime,
      const std::function<bool(char)> &validator,
      const std::function<Single(char)> &converter,
      const std::string &name)
  {
    std::string_view s = runtime.get_view();
    std::string failedMsg = std::string("Failed to parse ") + name;

    if (s.empty())
    {
      return {ParserFailed{failedMsg + ": end of input.", runtime.get_state().pos}};
    }
    else if (!validator(s.at(0)))
    {
      return {ParserFailed{failedMsg + ": not a " + name + ".", runtime.get_state().pos}};
    }

    ParserSucceeded<Single> r;
    r.parsed = converter(s.at(0));
    r.from = runtime.get_state().pos;
    r.to = runtime.get_state().pos + 1;
    return {r};
  }

  template <typename Dummy>
  ParserResult<std::string> parseLit(
      ParserRuntime &runtime,
      const std::string &litS)
  {
    std::string_view s = runtime.get_view();
    std::string failedMsg = std::string("Failed to parse ") + litS;

    if (s.size() < litS.size())
    {
      return {ParserFailed{failedMsg + ": end of input.", runtime.get_state().pos}};
    }
    else if (s.find(litS) != 0)
    {
      return {ParserFailed{failedMsg, runtime.get_state().pos}};
    }

    return ParserSucceeded<std::string>{litS, runtime.get_state().pos, runtime.get_state().pos + litS.size()};
  }

} // namespace core
} // namespace ps

#endif // PS_CORE_PARSERS_H
