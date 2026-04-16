#include <QString>
#include <QtTest>

#include <variant>
#include <iostream>

#include "common.h"
#include "ps/ps.h"

class PSTest : public QObject
{
  Q_OBJECT

public:
  PSTest();

private Q_SLOTS:

  void singleDigitParserTest();
  void singleDigitFromMiddleTest();
  void singleDigitFailureTest();
  void litParserTest();


  void digitCastTest();

  // void digitParserTest();
  // void lowerCaseCharParserTest();
  // void tryPTTest();
  // void upperCaseCharParserTest();
  // void symbolParserTest();
  // void manyCombinatorTest();
  // void parseFailureTest();
  // void tryThrowParserLTest();
  // void tryThrowParserTTest();
  // void safeVSTryPTest();

  // void bindPureTest();
  // void applicativeTest();
  // void sequencedParsersTest();
  // void forgetCombinatorsTest();
  // void seqCombinatorsTest();
  // void binarySeqCombinatorsTest();

  // void alt1Test();
  // void alt2Test();
  // void internalParsersTest();

  void parserRuntimeTest();
};

PSTest::PSTest()
{
}

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

void PSTest::singleDigitParserTest()
{
  using namespace ps;

  auto src = "1";
  std::string_view src_view(src);

  ParserRuntime runtime(src, State{0});
  ParserResult<Char> result = parse_with_runtime<Char>(runtime, digit);

  auto messages = runtime.get_messages();
  for (const auto &msg : messages)
  {
      std::cout << msg << "\n";
  }

  QVERIFY(isRight(result));
  auto parseSucceeded = getParseSucceeded(result);
  Char r = parseSucceeded.parsed;

  std::cout << "Final position: " << parseSucceeded.to << "\n";
  std::cout << "Parsed character: '" << r << "'\n";

  QVERIFY(r == '1');
}

void PSTest::singleDigitFromMiddleTest()
{
  using namespace ps;

  auto src = "a1b";
  std::string_view src_view(src);

  ParserRuntime runtime(src, State{0});
  ParserResult<Char> result = parse_with_runtime<Char>(runtime, digit, 1);

  auto messages = runtime.get_messages();
  for (const auto &msg : messages)
  {
      std::cout << msg << "\n";
  }

  QVERIFY(isRight(result));
  auto parseSucceeded = getParseSucceeded(result);
  Char r = parseSucceeded.parsed;

  std::cout << "Final position: " << parseSucceeded.to << "\n";
  std::cout << "Parsed character: '" << r << "'\n";

  QVERIFY(r == '1');
}

void PSTest::singleDigitFailureTest()
{
  using namespace ps;

  auto src = "a";
  std::string_view src_view(src);

  ParserRuntime runtime(src, State{0});
  ParserResult<Char> result = parse_with_runtime<Char>(runtime, digit);

  auto messages = runtime.get_messages();
  for (const auto &msg : messages)
  {
      std::cout << msg << "\n";
  }

  QVERIFY(isLeft(result));
  auto parseFailed = getParseFailed(result);
}

void PSTest::litParserTest()
{
    using namespace ps;

    auto src = "str12";
    std::string_view src_view(src);

    auto my_lit = parseLit("str");

    ParserRuntime runtime(src, State{0});
    ParserResult<std::string> result =
        parse_with_runtime<std::string>(runtime, my_lit);

    QVERIFY(isRight(result));
    std::string r = getParseSucceeded<std::string>(result).parsed;
    QVERIFY(r == "str");
}


void PSTest::digitCastTest()
{
  using namespace ps;

  std::function<bool(std::any)> cond = [](std::any any) {
      char ch = std::any_cast<char>(any);
      return ch >= '0' && ch <= '9';
  };

  std::function<int(char)> charToInt = [](char ch) -> int {
      return ch - '0';
  };

  ParserADT<char> digitADT =
  {
      ParseSymbolCond<char>{
          0,
          "",
          cond,
          [](const std::any& any)
          {
              char ch = std::any_cast<char>(any);
              int digitValue = ch - '0';
              return digitValue;
          }}
  };

  ParserADT<int> digitIntADT = methods_fmap<char, int>(charToInt, digitADT);

auto src = "1";
std::string_view src_view(src);

ParserL<int> digit_casted = fmap<Char, int>(charToInt, digit);


ParserRuntime runtime(src, State{0});
ParserResult<int> result = parse_with_runtime<int>(runtime, digit_casted);

auto messages = runtime.get_messages();
for (const auto &msg : messages)
{
    std::cout << msg << "\n";
}

QVERIFY(isRight(result));

}

// void PSTest::lowerCaseCharParserTest()
// {
//   using namespace ps;

//   ParserResult<Char> result = parse<Char>(lower, "abc");

//   QVERIFY(isRight(result));
//   Char r = getParseSucceeded<Char>(result).parsed;
//   QVERIFY(r == 'a');
// }

// void PSTest::tryPTTest()
// {
//   using namespace ps;

//   ParserResult<Char> result1 = parse<Char>({tryPT(digit)}, "1ab");
//   ParserResult<Char> result2 = parse<Char>({tryPT(digit)}, "ab");

//   QVERIFY(isRight(result1));
//   QVERIFY(isLeft(result2));
//   Char r1 = getParseSucceeded<Char>(result1).parsed;
//   QVERIFY(r1 == '1');
// }


// void PSTest::upperCaseCharParserTest()
// {
//   using namespace ps;

//   ParserResult<Char> result = parse<Char>(upper, "BCD");

//   QVERIFY(isRight(result));
//   Char r = getParseSucceeded<Char>(result).parsed;
//   QVERIFY(r == 'B');
// }

// void PSTest::symbolParserTest()
// {
//   using namespace ps;

//   ParserResult<Char> result = parse<Char>(symbol('B'), "BCD");

//   QVERIFY(isRight(result));
//   Char r = getParseSucceeded<Char>(result).parsed;
//   QVERIFY(r == 'B');
// }

// struct R2
// {
//   ps::Many<ps::Char> ds;
//   ps::Char ch;
// };

// void PSTest::manyCombinatorTest()
// {
//   using namespace ps;

//   ParserT<Many<Char>> p = manyPL<Char>(digitThrowPL);

//   ParserResult<Many<Char>> result = parse(p, "4321");

//   QVERIFY(isRight(result));
//   Many<Char> parsed = getParseSucceeded(result).parsed;
//   QVERIFY(parsed.size() == 4);
//   QVERIFY(parsed.front() == '4');
//   parsed.pop_front();
//   QVERIFY(parsed.front() == '3');
//   parsed.pop_front();
//   QVERIFY(parsed.front() == '2');
//   parsed.pop_front();
//   QVERIFY(parsed.front() == '1');
// }

// void PSTest::parseFailureTest()
// {
//   using namespace ps;

//   ParserResult<Char> result = parse<Char>(digit, "abc");

//   QVERIFY(isLeft(result));
//   QVERIFY(std::get<ParserFailed>(result).message == "Failed to parse digit: not a digit.");
// }

// void PSTest::tryThrowParserLTest()
// {
//   using namespace ps;

//   auto f = [](const ParserResult<Char> &)
//   {
//     return pure<ParserResult<Char>>(ParserSucceeded<Char>{'1'});
//   };

//   ParserT<ParserResult<Char>> pt = tryP(digitThrowPL);
//   auto pt2 = bind<ParserResult<Char>, ParserResult<Char>>(pt, f);

//   auto result = parse(pt2, "abc");

//   QVERIFY(isLeft(result));
// }

// /*

// -- f should fail with err1
// f = do
//     try (parserA >> parserB >> fail err1)
//         >>= \eRes -> fail err2

// -- g should fail with err2
// g = do
//     safe (parserA >> parserB >> fail err1)
//         >>= \eRes -> fail err2

// */

// void PSTest::safeVSTryPTest()
// {
//   using namespace ps;

//   auto f = [](ParserResult<Unit>)
//   {
//     throw std::runtime_error("err2");
//     return pure(unit);
//   };

//   ParserL<Unit> internalP =
//       bind<Char, Unit>(digitThrowPL, [](Char)
//                        { return bind<Char, Unit>(digitThrowPL, [](Char)
//                                                  {
//                 throw std::runtime_error("err1");
//                 return purePL(unit); }); });

//   ParserT<Unit> triedP = bind<ParserResult<Unit>, Unit>(tryP(internalP), f);
//   ParserT<Unit> safedP = bind<ParserResult<Unit>, Unit>(safeP(internalP), f);

//   auto result1 = parse(triedP, "123");
//   auto result2 = parse(safedP, "123");

//   QVERIFY(isLeft(result1));
//   QVERIFY(getParseFailed(result1).message == "err1");
//   QVERIFY(isLeft(result2));
//   QVERIFY(getParseFailed(result2).message == "err2");
// }

// struct R
// {
//   ps::Char dg0;
//   ps::Char ch1;
//   ps::Char ch2;
// };

// void PSTest::bindPureTest()
// {
//   using namespace ps;

//   auto p = bind<Char, R>(digit, [=](Char d1)
//                          { return pure<R>(R{d1, 'a', '0'}); });

//   ParserResult<R> result = parse(p, "1b2");

//   QVERIFY(isRight(result));
//   R r = getParseSucceeded<R>(result).parsed;
//   QVERIFY(r.dg0 == '1');
//   QVERIFY(r.ch1 == 'a');
//   QVERIFY(r.ch2 == '0');
// }

// void PSTest::applicativeTest()
// {
//   using namespace ps;

//   auto mkR = [](Char dg0, Char ch1, Char ch2)
//   { return R{dg0, ch1, ch2}; };

//   auto p = app<R, Char, Char, Char>(mkR, digit, lower, digit);

//   ParserResult<R> result = parse(p, "1b2");

//   QVERIFY(isRight(result));
//   R r = getParseSucceeded<R>(result).parsed;
//   QVERIFY(r.dg0 == '1');
//   QVERIFY(r.ch1 == 'b');
//   QVERIFY(r.ch2 == '2');
// }

// void PSTest::sequencedParsersTest()
// {
//   using namespace ps;

//   auto p = bind<Char, R>(digit, [=](Char dg0)
//                          { return bind<Char, R>(lower, [=](Char ch1)
//                                                 { return bind<Char, R>(symbol('2'), [=](Char ch2)
//                                                                        { return pure<R>(R{dg0, ch1, ch2}); }); }); });

//   ParserResult<R> result = parse(p, "1b2");

//   QVERIFY(isRight(result));
//   R r = getParseSucceeded<R>(result).parsed;
//   QVERIFY(r.dg0 == '1');
//   QVERIFY(r.ch1 == 'b');
//   QVERIFY(r.ch2 == '2');
// }

// void PSTest::forgetCombinatorsTest()
// {
//   using namespace ps;

//   auto fst = forgetFirst(letter, digit);
//   auto snd = forgetSecond(letter, digit);

//   ParserResult<Char> result1 = parse(fst, "a1");
//   ParserResult<Char> result2 = parse(snd, "a1");

//   QVERIFY(isRight(result1));
//   QVERIFY(isRight(result2));
//   QVERIFY(getParseSucceeded<Char>(result1).parsed == '1');
//   QVERIFY(getParseSucceeded<Char>(result2).parsed == 'a');
// }

// void PSTest::seqCombinatorsTest()
// {
//   using namespace ps;

//   ParserResult<Char> result1 = parse(seq(upper, lower), "Aa");
//   ParserResult<Char> result2 = parse(seq(upper, lower, digit), "Aa1");
//   ParserResult<Char> result3 = parse(seq(upper, lower, digit, symbol('!')), "Aa1!");

//   QVERIFY(isRight(result1));
//   QVERIFY(isRight(result2));
//   QVERIFY(isRight(result3));
//   QVERIFY(getParseSucceeded<Char>(result1).parsed == 'a');
//   QVERIFY(getParseSucceeded<Char>(result2).parsed == '1');
//   QVERIFY(getParseSucceeded<Char>(result3).parsed == '!');
// }

// void PSTest::binarySeqCombinatorsTest()
// {
//   using namespace ps;

//   ParserResult<Char> result1 = parse(upper >> lower, "Aa");
//   ParserResult<Char> result2 = parse(upper >> lower >> digit, "Aa1");
//   ParserResult<Char> result3 = parse(upper >> (lower << digit), "Aa1");

//   QVERIFY(isRight(result1));
//   QVERIFY(isRight(result2));
//   QVERIFY(isRight(result3));
//   QVERIFY(getParseSucceeded<Char>(result1).parsed == 'a');
//   QVERIFY(getParseSucceeded<Char>(result2).parsed == '1');
//   QVERIFY(getParseSucceeded<Char>(result3).parsed == 'a');
// }

// void PSTest::alt1Test()
// {
//   using namespace ps;

//   ParserResult<Char> result1 = parse(alt(upperPL, lowerPL), "A");
//   ParserResult<Char> result2 = parse(alt(upperPL, lowerPL), "a");

//   QVERIFY(isRight(result1));
//   QVERIFY(isRight(result2));
//   QVERIFY(getParseSucceeded(result1).parsed == 'A');
//   QVERIFY(getParseSucceeded(result2).parsed == 'a');
// }

// void PSTest::alt2Test()
// {
//   using namespace ps;

//   ParserResult<Char> result1 = parse(alt(upperThrowPL, lowerThrowPL), "A");
//   ParserResult<Char> result2 = parse(alt(upperThrowPL, lowerThrowPL), "a");

//   QVERIFY(isRight(result1));
//   QVERIFY(isRight(result2));
//   QVERIFY(getParseSucceeded(result1).parsed == 'A');
//   QVERIFY(getParseSucceeded(result2).parsed == 'a');
// }

// void PSTest::internalParsersTest()
// {
//   using namespace ps;

//   ParserL<Char> p1 =
//       bind<Char, Char>(upperThrowPL, [=](auto)
//                        { return bind<Char, Char>(lowerThrowPL, [=](auto)
//                                                  { return lowerThrowPL; }); });

//   ParserL<Char> p2 =
//       bind<Char, Char>(upperThrowPL, [=](auto)
//                        { return bind<Char, Char>(upperThrowPL, [=](auto)
//                                                  { return upperThrowPL; }); });

//   ParserT<Char> p =
//       bind<Char, Char>(digit,
//                        [=](auto)
//                        { return alt(p1, p2); });

//   ParserResult<Char> result = parse<Char>(p, "1AAA2");

//   printError(result);

//   QVERIFY(isRight(result));
//   Char ch = getParseSucceeded<Char>(result).parsed;
//   QVERIFY(ch == 'A');
// }

void PSTest::parserRuntimeTest()
{
  using namespace ps;

  std::string src = "hello world";
  State st{5};
  ParserRuntime runtime(src, st);

  // view starts at position 5 -> " world"
  std::string_view v = runtime.get_view();
  QVERIFY(v == std::string_view(" world"));

  // put_state to 2 -> "llo world"
  State s2{2};
  runtime.put_state(s2);
  QVERIFY(runtime.get_state().pos == 2);
  QVERIFY(runtime.get_view() == std::string_view("llo world"));
}

QTEST_APPLESS_MAIN(PSTest)

#include "tst_parsec.moc"
