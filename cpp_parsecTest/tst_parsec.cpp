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
  void onlyOneDigitTest();
  void singleDigitFromManyTest();
  void singleDigitFromMiddleTest();
  void onlyOneDigitFromMiddleTest();
  void singleDigitFailureTest();
  void litParserTest();


  void digitCastTest();

  void manyDigitsTest();
  void manyDigitsCastedTest();
  void manyParserCastedTest();

  void bindPureTest();

  void bindLeftIdentityTest();
  void bindRightIdentityTest();
  void bindAssociativityTest();
  void nestedBindSequenceTest();

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

  ParserRuntime runtime(src, State{});
  ParserResult<Char> result = parse_with_runtime<Char>(runtime, digit);

  QVERIFY(isRight(result));
  auto parseSucceeded = getParseSucceeded(result);
  Char r = parseSucceeded.parsed;

  QVERIFY(r == '1');
  QVERIFY(parseSucceeded.to == 1);
}

void PSTest::onlyOneDigitTest()
{
  using namespace ps;

  auto src = "12";
  ParserRuntime runtime(src, State{});
  ParserResult<Char> result = parse_with_runtime<Char>(runtime, digit);

  QVERIFY(isRight(result));
  auto parseSucceeded = getParseSucceeded(result);
  Char r = parseSucceeded.parsed;

  QVERIFY(r == '1');
  QVERIFY(parseSucceeded.to == 1);
}

void PSTest::singleDigitFromManyTest()
{
  using namespace ps;

  auto src = "123";
  ParserRuntime runtime(src, State{});
  ParserResult<Char> result = parse_with_runtime<Char>(runtime, digit, 0);

  QVERIFY(isRight(result));
  auto parseSucceeded = getParseSucceeded(result);
  Char r = parseSucceeded.parsed;

  QVERIFY(r == '1');
  QVERIFY(parseSucceeded.to == 1);
}

void PSTest::singleDigitFromMiddleTest()
{
  using namespace ps;

  auto src = "a1b";
  ParserRuntime runtime(src, State{});
  ParserResult<Char> result = parse_with_runtime<Char>(runtime, digit, 1);

  QVERIFY(isRight(result));
  auto parseSucceeded = getParseSucceeded(result);
  Char r = parseSucceeded.parsed;

  QVERIFY(r == '1');
  QVERIFY(parseSucceeded.to == 2);
}

void PSTest::onlyOneDigitFromMiddleTest()
{
  using namespace ps;

  auto src = "123";
  ParserRuntime runtime(src, State{});
  ParserResult<Char> result = parse_with_runtime<Char>(runtime, digit, 1);

  QVERIFY(isRight(result));
  auto parseSucceeded = getParseSucceeded(result);
  Char r = parseSucceeded.parsed;

  QVERIFY(r == '2');
  QVERIFY(parseSucceeded.to == 2);
}

void PSTest::singleDigitFailureTest()
{
  using namespace ps;

  auto src = "a";

  ParserRuntime runtime(src, State{});
  ParserResult<Char> result = parse_with_runtime<Char>(runtime, digit);

  QVERIFY(isLeft(result));
  auto parseFailed = getParseFailed(result);
}


void PSTest::litParserTest()
{
    using namespace ps;

    auto src = "str12";

    auto my_lit = parseLit("str");

    ParserRuntime runtime(src, State{});
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

  ParserL<int> digit_casted = fmap<Char, int>(charToInt, digit);
  ParserRuntime runtime(src, State{});
  ParserResult<int> result = parse_with_runtime<int>(runtime, digit_casted);

  QVERIFY(isRight(result));
  QVERIFY(getParseSucceeded(result).parsed == 1);
}

void PSTest::manyDigitsTest()
{
  using namespace ps;

  auto src = "123";
  ParserRuntime runtime(src, State{});

  ParserL<char> digitObj = digit;
  ParserL<Many<Char>> manyDigits = many<char>(&digitObj);   // TODO: check if it can be used twice without issues
  ParserResult<Many<Char>> result;

  result = parse_with_runtime<Many<Char>>(runtime, manyDigits);

  QVERIFY(isRight(result));
  Many<Char> parsed = getParseSucceeded(result).parsed;
  QVERIFY(parsed.size() == 3);
  QVERIFY(parsed.front() == '1');
  parsed.pop_front();
  QVERIFY(parsed.front() == '2');
  parsed.pop_front();
  QVERIFY(parsed.front() == '3');
}

void PSTest::manyDigitsCastedTest()
{
  using namespace ps;

  auto src = "123";
  ParserRuntime runtime(src, State{});

  ParserL<int> digitIntObj = fmap<Char, int>([](char ch) { return ch - '0'; }, digit);
  ParserL<Many<int>> manyDigitsInt = many<int>(&digitIntObj);

  ParserResult<Many<int>> result = parse_with_runtime<Many<int>>(runtime, manyDigitsInt);

  QVERIFY(isRight(result));
  Many<int> parsed = getParseSucceeded(result).parsed;
  QVERIFY(parsed.size() == 3);
  QVERIFY(parsed.front() == 1);
  parsed.pop_front();
  QVERIFY(parsed.front() == 2);
  parsed.pop_front();
  QVERIFY(parsed.front() == 3);
}

void PSTest::manyParserCastedTest()
{
  using namespace ps;

  auto src = "123";
  ParserRuntime runtime(src, State{});

  ParserL<char> digitObj = digit;
  ParserL<Many<Char>> manyDigits = many<char>(&digitObj);
  ParserL<Many<int>> manyDigitsInt = fmap<Many<Char>, Many<int>>(
      [](const Many<Char>& chars) {
          Many<int> ints;
          for (char ch : chars)
          {
              ints.push_back(ch - '0');
          }
          return ints;
      },
      manyDigits);

  ParserResult<Many<int>> result = parse_with_runtime<Many<int>>(runtime, manyDigitsInt);

  QVERIFY(isRight(result));
  Many<int> parsed = getParseSucceeded(result).parsed;
  QVERIFY(parsed.size() == 3);
  QVERIFY(parsed.front() == 1);
  parsed.pop_front();
  QVERIFY(parsed.front() == 2);
  parsed.pop_front();
  QVERIFY(parsed.front() == 3);
}

struct R
{
  ps::Char dg0;
  ps::Char ch1;
  ps::Char ch2;
};

void PSTest::bindPureTest()
{
  using namespace ps;

  ParserL<R> p = bind<Char, R>(digit, [=](Char d1) { return pure<R>(R{d1, 'a', '0'}); });

  auto src = "242fddvf";
  ParserRuntime runtime(src, State{});
  ParserResult<R> result = parse_with_runtime<R>(runtime, p, 0);

  QVERIFY(isRight(result));
  R r = getParseSucceeded<R>(result).parsed;
  QVERIFY(r.dg0 == '2');
  QVERIFY(r.ch1 == 'a');
  QVERIFY(r.ch2 == '0');
}

void PSTest::bindLeftIdentityTest()
{
  using namespace ps;

  auto f = [](int x) { return pure<std::string>(std::to_string(x)); };

  ParserL<std::string> left = bind<int, std::string>(pure<int>(5), f);
  ParserL<std::string> right = f(5);

  ParserRuntime runtime("", State{});
  ParserResult<std::string> rLeft = parse_with_runtime<std::string>(runtime, left);
  ParserResult<std::string> rRight = parse_with_runtime<std::string>(runtime, right);

  QVERIFY(isRight(rLeft));
  QVERIFY(isRight(rRight));
  QVERIFY(getParseSucceeded(rLeft).parsed == getParseSucceeded(rRight).parsed);
  QVERIFY(getParseSucceeded(rLeft).to == getParseSucceeded(rRight).to);
}

void PSTest::bindRightIdentityTest()
{
  using namespace ps;

  ParserL<Char> rightId = bind<Char, Char>(digit, [](Char c) { return pure<Char>(c); });

  ParserRuntime runtime("1", State{});
  ParserResult<Char> rOrig = parse_with_runtime<Char>(runtime, digit);
  ParserResult<Char> rRight = parse_with_runtime<Char>(runtime, rightId);

  QVERIFY(isRight(rOrig));
  QVERIFY(isRight(rRight));
  QVERIFY(getParseSucceeded(rOrig).parsed == getParseSucceeded(rRight).parsed);
  QVERIFY(getParseSucceeded(rOrig).to == getParseSucceeded(rRight).to);
}

void PSTest::bindAssociativityTest()
{
  using namespace ps;

  auto f = [](Char c) { return pure<int>(static_cast<int>(c - '0')); };
  auto g = [](int v) { return pure<std::string>(std::to_string(v)); };

  ParserL<std::string> left = bind<int, std::string>(bind<Char, int>(digit, f), g);
  ParserL<std::string> right = bind<Char, std::string>(digit, [=](Char c) { return bind<int, std::string>(f(c), g); });

  ParserRuntime runtime("7", State{});
  ParserResult<std::string> rLeft = parse_with_runtime<std::string>(runtime, left);
  ParserResult<std::string> rRight = parse_with_runtime<std::string>(runtime, right);

  QVERIFY(isRight(rLeft));
  QVERIFY(isRight(rRight));
  QVERIFY(getParseSucceeded(rLeft).parsed == getParseSucceeded(rRight).parsed);
  QVERIFY(getParseSucceeded(rLeft).to == getParseSucceeded(rRight).to);
}

void PSTest::nestedBindSequenceTest()
{
  using namespace ps;

  // Read three digits in sequence using nested binds and return them as R
  ParserL<R> seq = bind<Char, R>(digit, [=](Char d1) {
    return bind<Char, R>(digit, [=](Char d2) {
      return bind<Char, R>(digit, [=](Char d3) {
        return pure<R>(R{d1, d2, d3});
      });
    });
  });

  ParserRuntime runtime("123", State{});
  ParserResult<R> res = parse_with_runtime<R>(runtime, seq, 0);

  QVERIFY(isRight(res));
  R out = getParseSucceeded<R>(res).parsed;
  QVERIFY(out.dg0 == '1');
  QVERIFY(out.ch1 == '2');
  QVERIFY(out.ch2 == '3');
  QVERIFY(getParseSucceeded<R>(res).to == 3);
}

void PSTest::parserRuntimeTest()
{
  using namespace ps;

  std::string src = "hello world";
  State st{5};
  ParserRuntime runtime(src, st);
  QVERIFY(runtime.get_state().data == 5);

  // put_state to 2 -> "llo world"
  State s2{2};
  runtime.put_state(s2);
  QVERIFY(runtime.get_state().data == 2);
  QVERIFY(runtime.get_view() == std::string_view("hello world"));
}

QTEST_APPLESS_MAIN(PSTest)

#include "tst_parsec.moc"
