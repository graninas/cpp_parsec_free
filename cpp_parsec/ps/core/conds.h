#ifndef PS_CORE_CONDS_H
#define PS_CORE_CONDS_H

#include <functional>

namespace ps {
namespace core {

  std::function<bool(char)> chEq(char ch)
  {
      return [=](char ch1) { return ch1 == ch; };
  }

  const auto isLower = [](char ch)
  {
      return ch >= 'a' && ch <= 'z';
  };

  const auto isUpper = [](char ch)
  {
      return ch >= 'A' && ch <= 'Z';
  };

  const auto isAlpha = [](char ch)
  {
      return isLower(ch) || isUpper(ch);
  };

  const auto isDigit = [](char ch)
  {
      return ch >= '0' && ch <= '9';
  };

  const auto isAlphanum = [](char ch)
  {
      return isAlpha(ch) || isDigit(ch);
  };

  const auto isSpace = [](char ch)
  {
      return ch == ' ';
  };

  const auto isEol = [](char ch)
  {
      return ch == '\n';
  };

  const auto isCr = [](char ch)
  {
      return ch == '\r';
  };

} // namespace core
} // namespace ps

#endif // PS_CORE_CONDS_H
