#ifndef PS_CORE_METHODS_ADT_H
#define PS_CORE_METHODS_ADT_H

#include "../types.h"

namespace ps
{
namespace core
{

// PS methods

// TODO: `next` seems not needed for church
// TODO: `next` can be probably moved to ParerMethods or something

template <typename Next, template <typename> class Parser>
struct ParseSymbolCond
{
  std::function<bool(Any)> validator;
  std::function<Next(Any)> next;
};

template <typename Next, template <typename> class Parser>
struct ParseMany
{
  std::shared_ptr<Parser<Any>> rawParser;   // Parser is always the same
  std::function<Next(std::list<Any>)> next;
};

template <typename Next, template <typename> class Parser>
struct TryOrErrorParser
{
  std::shared_ptr<Parser<Any>> rawParser;   // Parser is always the same
  std::function<Next(ParserResult<Any>)> next;
};

template <typename Next, template <typename> class Parser>
struct AltParser
{
  std::shared_ptr<Parser<Any>> p;   // Parser is always the same
  std::shared_ptr<Parser<Any>> q;   // Parser is always the same
  std::function<Next(Any)> next;
};

template <typename Next, template <typename> class Parser>
struct LazyParser
{
  std::function<Parser<Any>()> parserFactory;   // Factory is always the same
  std::function<Next(Any)> next;
};

template <typename Next, template <typename> class Parser>
struct ParseLit
{
  std::string s;
  std::function<Next(std::string)> next;
};

template <typename Next, template <typename> class Parser>
struct GetSt
{
  std::function<Next(State)> next;
};

template <typename Next, template <typename> class Parser>
struct PutSt
{
    State st;
    std::function<Next(Unit)> next;
};


template <class Ret, template <typename> class Parser>
struct ParserMethods
{
  using ResultType = Ret;
  std::variant<
      ParseSymbolCond<Ret, Parser>,
      ParseMany<Ret, Parser>,
      ParseLit<Ret, Parser>,
      GetSt<Ret, Parser>,
      PutSt<Ret, Parser>,
      TryOrErrorParser<Ret, Parser>,
      AltParser<Ret, Parser>,
      LazyParser<Ret, Parser>>
      psf;
};

} // namespace core
} // namespace ps

#endif // PS_CORE_PARSER_ADT_H
