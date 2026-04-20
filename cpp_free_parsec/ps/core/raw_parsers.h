#ifndef PS_CORE_RAW_PARSERS_H
#define PS_CORE_RAW_PARSERS_H

#include <string>
#include <functional>

#include "types.h"

namespace ps
{
namespace core
{

  template <typename Dummy>
  ParserResult<char> parseSingle(
      ParserRuntime &runtime,
      Pos from,
      const std::function<bool(char)> &validator)
  {
    std::string_view s = runtime.getView().substr(from);

    if (s.empty())
    {
      std::string failedMsg = std::string("Failed to parse: end of input.");
      return {ParserFailed{failedMsg, from}};
    }
    else if (!validator(s.at(0)))
    {
      std::string failedMsg = std::string("Failed to parse: '") + s.at(0) + "' does not satisfy the condition.";
      return {ParserFailed{failedMsg, from}};
    }

    ParserSucceeded<char> r;
    r.parsed = s.at(0);
    r.from = from;
    r.to = from + 1;
    runtime.pushMessage(std::string("Parsed: '") + s.at(0) + "'.");
    return r;
  }

  // Dummy template parameter is for keeping it in the header file and not defining it in a .cpp file, since it's a template function.
  template <typename Dummy>
  ParserResult<std::string> parseLit(
      ParserRuntime &runtime,
      Pos from,
      const std::string &litS)
  {
    std::string_view s = runtime.getView().substr(from);

    if (s.size() < litS.size())
    {
      std::string failedMsg = std::string("Failed to parse lit:") + litS + ": end of input.";
      return {ParserFailed{failedMsg, from}};
    }
    else if (s.substr(0, litS.size()) != litS)
    {
      std::string failedMsg = std::string("Failed to parse lit: ") + litS;
      return {ParserFailed{failedMsg, from}};
    }

    ParserSucceeded<std::string> r;
    r.parsed = litS;
    r.from = from;
    r.to = from + litS.size();
    runtime.pushMessage(std::string("Parsed lit: ") + litS + ".");
    return r;
  }



} // namespace core
} // namespace ps

#endif // PS_CORE_RAW_PARSERS_H
