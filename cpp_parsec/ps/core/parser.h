#ifndef PS_CORE_PARSER_H
#define PS_CORE_PARSER_H

#include "types.h"
#include "runtime.h"
#include "conds.h"
#include "parser/adt.h"
#include "parser/functor.h"
#include "parser/bind.h"
#include "interpreter.h"

namespace ps
{
namespace core
{

// New design

// Interface

template <typename A>
ParserResult<A> parse(
        const ParserL<A>& pst,
        const std::string& s,
        Pos from = 0)
{
    ParserRuntime runtime (s, State{});
    return runParser(runtime, pst, from);
}

template <typename A>
ParserResult<A> parse_with_runtime(
    ParserRuntime &runtime,
    const ParserL<A> &pst,
    Pos from = 0)
{
  return runParser(runtime, pst, from);
}

// Parsers

ParserL<Char> parseSymbolCond(
        const std::string& name,
        const std::function<bool(char)>& cond)
{
  std::function<bool(char)> cond_copy = cond;

  std::function<bool(Any)> condAny = [=](const Any& any)  {
      char ch = std::any_cast<char>(any);
      return cond_copy(ch);
  };

    return make_free(ParseSymbolCond<ParserL<Char>>{
                      name,
                      condAny,
                      [](const Any& any)  {
                          char ch = std::any_cast<char>(any);
                          {
                              return make_pure(ch);
                          }
                      }
                });
}

ParserL<std::string> parseLit(const std::string& s)
{
    return make_free(ParseLit<ParserL<std::string>>{
                      s,
                      [](const std::string& resS) {
                          return make_pure(resS);
                      }
                });
}

const ParserL<Char> digit = parseSymbolCond("digit", isDigit);
const ParserL<Char> upper = parseSymbolCond("upper", isUpper);
const ParserL<Char> lower = parseSymbolCond("lower", isLower);
const ParserL<Char> alpha = parseSymbolCond("alpha", isAlpha);
const ParserL<Char> alphanum = parseSymbolCond("alphanum", isAlphanum);
const ParserL<Char> space = parseSymbolCond("space", isSpace);
const ParserL<Char> anyChar = parseSymbolCond("any char", [](char) { return true; });
const ParserL<Char> eol = parseSymbolCond("eol", isEol);
const ParserL<Char> cr = parseSymbolCond("cr", isCr);

template <typename A>
ParserL<Many<A>> many(ParserL<A>* p)
{
  auto pCopy = std::make_shared<ParserL<Any>>(
      fmap<A, Any>([](const A &a)
                   { return a; }, *p));

  return make_free(ParseMany<ParserL<Many<A>>>{
      pCopy,
      [](const std::list<Any> &resList)
      {
        Many<A> res;
        for (const Any &any : resList)
        {
          A a = std::any_cast<A>(any);
          res.push_back(a);
        }
        return make_pure(res);
      }});
}





} // namespace core
} // namespace ps

#endif // PS_CORE_PARSER_H
