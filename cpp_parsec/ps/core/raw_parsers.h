#ifndef PS_RAW_PARSERS_H
#define PS_RAW_PARSERS_H

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
      const std::function<bool(char)> &validator,
      const std::string &name)
  {
    std::string_view s = runtime.get_view().substr(from);

    if (s.empty())
    {
      std::string failedMsg = std::string("Failed to parse ") + name + ": end of input.";
      runtime.push_message(failedMsg);
      return {ParserFailed{failedMsg, from}};
    }
    else if (!validator(s.at(0)))
    {
      std::string failedMsg = std::string("Failed to parse ") + name + ": '" + s.at(0) + "' does not satisfy the condition.";
      runtime.push_message(failedMsg);
      return {ParserFailed{failedMsg, from}};
    }

    ParserSucceeded<char> r;
    r.parsed = s.at(0);
    r.from = from;
    r.to = from + 1;
    runtime.push_message(std::string("Parsed ") + name + ": '" + s.at(0) + "'.");
    return r;
  }

  // Dummy template parameter is for keeping it in the header file and not defining it in a .cpp file, since it's a template function.
  template <typename Dummy>
  ParserResult<std::string> parseLit(
      ParserRuntime &runtime,
      Pos from,
      const std::string &litS)
  {
    std::string_view s = runtime.get_view().substr(from);

    if (s.size() < litS.size())
    {
      std::string failedMsg = std::string("Failed to parse lit:") + litS + ": end of input.";
      runtime.push_message(failedMsg);
      return {ParserFailed{failedMsg, from}};
    }
    else if (s.substr(0, litS.size()) != litS)
    {
      std::string failedMsg = std::string("Failed to parse lit: ") + litS;
      runtime.push_message(failedMsg);
      return {ParserFailed{failedMsg, from}};
    }

    ParserSucceeded<std::string> r;
    r.parsed = litS;
    r.from = from;
    r.to = from + litS.size();
    runtime.push_message(std::string("Parsed lit: ") + litS + ".");
    return r;
  }



} // namespace core
} // namespace ps

#endif // PS_RAW_PARSERS_H
