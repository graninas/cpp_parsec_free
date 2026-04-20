#include <QString>
#include <QtTest>

#include <variant>
#include <iostream>

#include "common.h"

#include "tst_free_parsec.h"
#include <QTest>

FreeParsecTest::FreeParsecTest(QObject *parent) : QObject(parent) {}

void FreeParsecTest::initTestCase()
{
}

void FreeParsecTest::cleanupTestCase()
{
}

void FreeParsecTest::tryCombinatorTest()
{
  using namespace ps;

  // Case 1: try allows backtracking on failure after consuming input
  // Try to parse "ab" or "ac" from "ac". Without try, the first parser would consume 'a' and fail, blocking the second.
  auto src = "ac";
  ParserRuntime runtime1(src, State{});
  Parser<std::string> p1 = alt(
      try_(seq(parseLit("a"), parseLit("b"))), // will consume 'a', fail on 'b'
      seq(parseLit("a"), parseLit("c"))        // should succeed due to try
  );
  ParserResult<std::string> r1 = parseWithRuntime<std::string>(runtime1, p1);

  // std::cout << "Test case 1: try allows backtracking on failure after consuming input" << std::endl;
  // printMessages(runtime1);

  QVERIFY(isRight(r1));
  QVERIFY(getParseSucceeded(r1).parsed == "c");
  QVERIFY(getParseSucceeded(r1).from == 0);
  QVERIFY(getParseSucceeded(r1).to == 2);

  // Case 2: try does not interfere with success
  ParserRuntime runtime2("ab", State{});
  Parser<std::string> p2 = try_(parseLit("ab"));
  ParserResult<std::string> r2 = parseWithRuntime<std::string>(runtime2, p2);

  // std::cout << "Test case 2: try does not interfere with success" << std::endl;
  // printMessages(runtime2);

  QVERIFY(isRight(r2));
  QVERIFY(getParseSucceeded(r2).parsed == "ab");

  // Case 3: try on a parser that fails without consuming input (should be a no-op)
  ParserRuntime runtime3("xy", State{});
  Parser<std::string> p3 = try_(parseLit("z"));
  ParserResult<std::string> r3 = parseWithRuntime<std::string>(runtime3, p3);

  // std::cout << "Test case 3: try on a parser that fails without consuming input (should be a no-op)" << std::endl;
  // printMessages(runtime3);

  QVERIFY(isLeft(r3));
  // Input position should not advance
  QVERIFY(getParseFailed(r3).at == 0);

  // Case 4: try with alternative, first succeeds, second not tried
  ParserRuntime runtime4("ab", State{});
  Parser<std::string> p4 = alt(try_(parseLit("ab")), parseLit("a"));
  ParserResult<std::string> r4 = parseWithRuntime<std::string>(runtime4, p4);

  // std::cout << "Test case 4: try with alternative, first succeeds, second not tried" << std::endl;
  // printMessages(runtime4);

  QVERIFY(isRight(r4));
  QVERIFY(getParseSucceeded(r4).parsed == "ab");
}

void FreeParsecTest::choiceCombinatorTest()
{
  using namespace ps;

  // Test 1: Simple choice between two literals
  Parser<std::string> parser1 = choice(parseLit("A"), parseLit("B"));
  ParserRuntime runtime1("A", State{});
  auto result1 = parseWithRuntime(runtime1, parser1);
  QVERIFY(isRight(result1));
  QVERIFY(getParseSucceeded(result1).parsed == "A");

  // Test 2: Choice with the second option succeeding
  ParserRuntime runtime2("B", State{});
  auto result2 = parseWithRuntime(runtime2, parser1);
  QVERIFY(isRight(result2));
  QVERIFY(getParseSucceeded(result2).parsed == "B");

  // Test 3: Choice with no options succeeding
  ParserRuntime runtime3("C", State{});
  auto result3 = parseWithRuntime(runtime3, parser1);
  QVERIFY(isLeft(result3));

  // Test 4: Choice with multiple options
  Parser<std::string> parser2 = choice(parseLit("X"), parseLit("Y"), parseLit("Z"));
  ParserRuntime runtime4("Y", State{});
  auto result4 = parseWithRuntime(runtime4, parser2);
  QVERIFY(isRight(result4));
  QVERIFY(getParseSucceeded(result4).parsed == "Y");

  // Test 5: Nested choice
  Parser<std::string> parser3 = choice(parser1, parser2);
  ParserRuntime runtime5("Z", State{});
  auto result5 = parseWithRuntime(runtime5, parser3);
  QVERIFY(isRight(result5));
  QVERIFY(getParseSucceeded(result5).parsed == "Z");
}

void FreeParsecTest::singleDigitParserTest()
{
  using namespace ps;

  auto src = "1";
  std::string_view src_view(src);

  ParserRuntime runtime(src, State{});
  ParserResult<Char> result = parseWithRuntime<Char>(runtime, digit);

  QVERIFY(isRight(result));
  auto parseSucceeded = getParseSucceeded(result);
  Char r = parseSucceeded.parsed;

  QVERIFY(r == '1');
  QVERIFY(parseSucceeded.to == 1);
}

void FreeParsecTest::onlyOneDigitTest()
{
  using namespace ps;

  auto src = "12";
  ParserRuntime runtime(src, State{});
  ParserResult<Char> result = parseWithRuntime<Char>(runtime, digit);

  QVERIFY(isRight(result));
  auto parseSucceeded = getParseSucceeded(result);
  Char r = parseSucceeded.parsed;

  QVERIFY(r == '1');
  QVERIFY(parseSucceeded.to == 1);
}

void FreeParsecTest::singleDigitFromManyTest()
{
  using namespace ps;

  auto src = "123";
  ParserRuntime runtime(src, State{});
  ParserResult<Char> result = parseWithRuntime<Char>(runtime, digit, 0);

  QVERIFY(isRight(result));
  auto parseSucceeded = getParseSucceeded(result);
  Char r = parseSucceeded.parsed;

  QVERIFY(r == '1');
  QVERIFY(parseSucceeded.to == 1);
}

void FreeParsecTest::singleDigitFromMiddleTest()
{
  using namespace ps;

  auto src = "a1b";
  ParserRuntime runtime(src, State{});
  ParserResult<Char> result = parseWithRuntime<Char>(runtime, digit, 1);

  QVERIFY(isRight(result));
  auto parseSucceeded = getParseSucceeded(result);
  Char r = parseSucceeded.parsed;

  QVERIFY(r == '1');
  QVERIFY(parseSucceeded.to == 2);
}

void FreeParsecTest::onlyOneDigitFromMiddleTest()
{
  using namespace ps;

  auto src = "123";
  ParserRuntime runtime(src, State{});
  ParserResult<Char> result = parseWithRuntime<Char>(runtime, digit, 1);

  QVERIFY(isRight(result));
  auto parseSucceeded = getParseSucceeded(result);
  Char r = parseSucceeded.parsed;

  QVERIFY(r == '2');
  QVERIFY(parseSucceeded.to == 2);
}

void FreeParsecTest::singleDigitFailureTest()
{
  using namespace ps;

  auto src = "a";

  ParserRuntime runtime(src, State{});
  ParserResult<Char> result = parseWithRuntime<Char>(runtime, digit);

  QVERIFY(isLeft(result));
  auto parseFailed = getParseFailed(result);
}


void FreeParsecTest::litParserTest()
{
    using namespace ps;

    auto src = "str12";

    auto my_lit = parseLit("str");

    ParserRuntime runtime(src, State{});
    ParserResult<std::string> result =
        parseWithRuntime<std::string>(runtime, my_lit);

    QVERIFY(isRight(result));
    std::string r = getParseSucceeded<std::string>(result).parsed;
    QVERIFY(r == "str");
}


void FreeParsecTest::digitCastTest()
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
          cond,
          [](const std::any& any)
          {
              char ch = std::any_cast<char>(any);
              int digitValue = ch - '0';
              return digitValue;
          }}
  };

  ParserADT<int> digitIntADT = fmapMethods<char, int>(charToInt, digitADT);

  auto src = "1";

  Parser<int> digit_casted = fmap<Char, int>(charToInt, digit);
  ParserRuntime runtime(src, State{});
  ParserResult<int> result = parseWithRuntime<int>(runtime, digit_casted);

  QVERIFY(isRight(result));
  QVERIFY(getParseSucceeded(result).parsed == 1);
}

void FreeParsecTest::manyTest()
{
  using namespace ps;

  auto src = "123";
  ParserRuntime runtime(src, State{});

  // many: run parser zero or more times and return list of results
  // N.B. many may hang if the raw parser can succeed without consuming any input.
  //   ParserRuntime has a safety check to prevent infinite loops in the many combinator.
  //   You can configure the safety check threshold in the ParserRuntime constructor, or you can make sure to never use many with a raw parser that can succeed without consuming input.
  //   This is a common issue with `many` combinators in parser combinator libraries, and it's important to be aware of it when using many.
  Parser<Many<Char>> manyDigits = many<char>(digit);
  ParserResult<Many<Char>> result;

  result = parseWithRuntime<Many<Char>>(runtime, manyDigits);

  QVERIFY(isRight(result));
  Many<Char> parsed = getParseSucceeded(result).parsed;
  QVERIFY(parsed.size() == 3);
  QVERIFY(parsed.front() == '1');
  parsed.pop_front();
  QVERIFY(parsed.front() == '2');
  parsed.pop_front();
  QVERIFY(parsed.front() == '3');
}

void FreeParsecTest::manyDigitsCastedTest()
{
  using namespace ps;

  auto src = "123";
  ParserRuntime runtime(src, State{});

  Parser<int> digitIntObj = fmap<Char, int>([](char ch) { return ch - '0'; }, digit);
  Parser<Many<int>> manyDigitsInt = many<int>(digitIntObj);

  ParserResult<Many<int>> result = parseWithRuntime<Many<int>>(runtime, manyDigitsInt);

  QVERIFY(isRight(result));
  Many<int> parsed = getParseSucceeded(result).parsed;
  QVERIFY(parsed.size() == 3);
  QVERIFY(parsed.front() == 1);
  parsed.pop_front();
  QVERIFY(parsed.front() == 2);
  parsed.pop_front();
  QVERIFY(parsed.front() == 3);
}

void FreeParsecTest::manyParserCastedTest()
{
  using namespace ps;

  auto src = "123";
  ParserRuntime runtime(src, State{});

  Parser<Many<Char>> manyDigits = many<char>(digit);
  Parser<Many<int>> manyDigitsInt = fmap<Many<Char>, Many<int>>(
      [](const Many<Char>& chars) {
          Many<int> ints;
          for (char ch : chars)
          {
              ints.push_back(ch - '0');
          }
          return ints;
      },
      manyDigits);

  ParserResult<Many<int>> result = parseWithRuntime<Many<int>>(runtime, manyDigitsInt);

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

void FreeParsecTest::bindPureTest()
{
  using namespace ps;

  Parser<R> p = bind<Char, R>(digit, [=](Char d1) { return pure<R>(R{d1, 'a', '0'}); });

  auto src = "242fddvf";
  ParserRuntime runtime(src, State{});
  ParserResult<R> result = parseWithRuntime<R>(runtime, p, 0);

  QVERIFY(isRight(result));
  R r = getParseSucceeded<R>(result).parsed;
  QVERIFY(r.dg0 == '2');
  QVERIFY(r.ch1 == 'a');
  QVERIFY(r.ch2 == '0');
}

void FreeParsecTest::applicativeCombinatorTest()
{
  using namespace ps;

  // Parser that produces a function
  auto addParser = pure<std::function<int(int)>>([](int x)
                                                 { return x + 10; });

  // Parser that produces a value
  auto valueParser = pure<int>(5);

  // Combine parsers using applicative
  auto combinedParser = applicative(addParser, valueParser);

  // Run the parser
  ParserRuntime runtime("", State{});
  ParserResult<int> result = parseWithRuntime<int>(runtime, combinedParser);

  // Verify the result
  QVERIFY(isRight(result));
  int parsedValue = getParseSucceeded(result).parsed;
  QVERIFY(parsedValue == 15);
}

void FreeParsecTest::applicativeCombinatorTestWithStrings()
{
  using namespace ps;

  // Parser that produces a function
  auto concatParser = pure<std::function<std::string(std::string)>>([](const std::string &s)
                                                                    { return s + " World"; });

  // Parser that produces a value
  auto valueParser = pure<std::string>("Hello");

  // Combine parsers using applicative
  auto combinedParser = applicative(concatParser, valueParser);

  // Run the parser
  ParserRuntime runtime("", State{});
  ParserResult<std::string> result = parseWithRuntime<std::string>(runtime, combinedParser);

  // Verify the result
  QVERIFY(isRight(result));
  std::string parsedValue = getParseSucceeded(result).parsed;
  QVERIFY(parsedValue == "Hello World");
}

void FreeParsecTest::bindLeftIdentityTest()
{
  using namespace ps;

  auto f = [](int x) { return pure<std::string>(std::to_string(x)); };

  Parser<std::string> left = bind<int, std::string>(pure<int>(5), f);
  Parser<std::string> right = f(5);

  ParserRuntime runtime("", State{});
  ParserResult<std::string> rLeft = parseWithRuntime<std::string>(runtime, left);
  ParserResult<std::string> rRight = parseWithRuntime<std::string>(runtime, right);

  QVERIFY(isRight(rLeft));
  QVERIFY(isRight(rRight));
  QVERIFY(getParseSucceeded(rLeft).parsed == getParseSucceeded(rRight).parsed);
  QVERIFY(getParseSucceeded(rLeft).to == getParseSucceeded(rRight).to);
}

void FreeParsecTest::bindRightIdentityTest()
{
  using namespace ps;

  Parser<Char> rightId = bind<Char, Char>(digit, [](Char c) { return pure<Char>(c); });

  ParserRuntime runtime("1", State{});
  ParserResult<Char> rOrig = parseWithRuntime<Char>(runtime, digit);
  ParserResult<Char> rRight = parseWithRuntime<Char>(runtime, rightId);

  QVERIFY(isRight(rOrig));
  QVERIFY(isRight(rRight));
  QVERIFY(getParseSucceeded(rOrig).parsed == getParseSucceeded(rRight).parsed);
  QVERIFY(getParseSucceeded(rOrig).to == getParseSucceeded(rRight).to);
}

void FreeParsecTest::bindAssociativityTest()
{
  using namespace ps;

  auto f = [](Char c) { return pure<int>(static_cast<int>(c - '0')); };
  auto g = [](int v) { return pure<std::string>(std::to_string(v)); };

  Parser<std::string> left = bind<int, std::string>(bind<Char, int>(digit, f), g);
  Parser<std::string> right = bind<Char, std::string>(digit, [=](Char c) { return bind<int, std::string>(f(c), g); });

  ParserRuntime runtime("7", State{});
  ParserResult<std::string> rLeft = parseWithRuntime<std::string>(runtime, left);
  ParserResult<std::string> rRight = parseWithRuntime<std::string>(runtime, right);

  QVERIFY(isRight(rLeft));
  QVERIFY(isRight(rRight));
  QVERIFY(getParseSucceeded(rLeft).parsed == getParseSucceeded(rRight).parsed);
  QVERIFY(getParseSucceeded(rLeft).to == getParseSucceeded(rRight).to);
}

void FreeParsecTest::nestedBindSequenceTest()
{
  using namespace ps;

  // Read three digits in sequence using nested binds and return them as R
  Parser<R> inSequence = bind<Char, R>(digit, [=](Char d1) {
    return bind<Char, R>(digit, [=](Char d2) {
      return bind<Char, R>(digit, [=](Char d3) {
        return pure<R>(R{d1, d2, d3});
      });
    });
  });

  ParserRuntime runtime("123", State{});
  ParserResult<R> res = parseWithRuntime<R>(runtime, inSequence, 0);

  QVERIFY(isRight(res));
  R out = getParseSucceeded<R>(res).parsed;
  QVERIFY(out.dg0 == '1');
  QVERIFY(out.ch1 == '2');
  QVERIFY(out.ch2 == '3');
  QVERIFY(getParseSucceeded<R>(res).to == 3);
}

void FreeParsecTest::parserRuntimeTest()
{
  using namespace ps;

  std::string src = "hello world";
  State st{5};
  ParserRuntime runtime(src, st);
  QVERIFY(runtime.getState().data == 5);

  // put_state to 2 -> "llo world"
  State s2{2};
  runtime.putState(s2);
  QVERIFY(runtime.getState().data == 2);
  QVERIFY(runtime.getView() == std::string_view("hello world"));
}

void FreeParsecTest::seqTest()
{
  using namespace ps;

  // seq: run two parsers and return second
  auto src = "12";
  ParserRuntime runtime(src, State{});

  Parser<Char> p = seq<Char, Char>(digit, digit);
  ParserResult<Char> r = parseWithRuntime<Char>(runtime, p);

  QVERIFY(isRight(r));
  QVERIFY(getParseSucceeded(r).parsed == '2');
  QVERIFY(getParseSucceeded(r).to == 2);
}

void FreeParsecTest::leftRightTest()
{
  using namespace ps;

  auto src = "1a";
  ParserRuntime runtime(src, State{});

  // left: run two parsers and return first
  // right: run two parsers and return second

  Parser<Char> leftP = left<Char, std::string>(digit, parseLit("a"));
  ParserResult<Char> rl = parseWithRuntime<Char>(runtime, leftP);
  QVERIFY(isRight(rl));
  QVERIFY(getParseSucceeded(rl).parsed == '1');

  Parser<std::string> rightP = right<Char, std::string>(digit, parseLit("a"));
  ParserResult<std::string> rr = parseWithRuntime<std::string>(runtime, rightP);
  QVERIFY(isRight(rr));
  QVERIFY(getParseSucceeded(rr).parsed == "a");
}

void FreeParsecTest::many1Test()
{
  using namespace ps;

  auto src = "123";
  ParserRuntime runtime(src, State{});

  // many1: run parser one or more times and return list of results
  // N.B. many1 will hang if the parser can succeed without consuming input, so we test with digit which always consumes input on success
  // See manyTest for more discussion on this issue with many1 and many combinators in general.

  Parser<Char> digitObj = digit;
  Parser<Many<Char>> p = many1<Char>(digitObj);
  ParserResult<Many<Char>> r = parseWithRuntime<Many<Char>>(runtime, p);

  QVERIFY(isRight(r));
  Many<Char> parsed = getParseSucceeded(r).parsed;
  QVERIFY(parsed.size() == 3);
  QVERIFY(parsed.front() == '1');
  parsed.pop_front();
  QVERIFY(parsed.front() == '2');
  parsed.pop_front();
  QVERIFY(parsed.front() == '3');
}

void FreeParsecTest::sepBy1Test()
{
  using namespace ps;

  auto src = "1,2,3";
  ParserRuntime runtime(src, State{});

  // sepBy1: run parser one or more times separated by sep parser, and return list of results from main parser
  Parser<Many<Char>> p = sepBy1<Char, std::string>(digit, parseLit(","));
  ParserResult<Many<Char>> r = parseWithRuntime<Many<Char>>(runtime, p);

  QVERIFY(isRight(r));
  Many<Char> parsed = getParseSucceeded(r).parsed;
  QVERIFY(parsed.size() == 3);
  QVERIFY(parsed.front() == '1');
  parsed.pop_front();
  QVERIFY(parsed.front() == '2');
  parsed.pop_front();
  QVERIFY(parsed.front() == '3');
}

void FreeParsecTest::betweenTest()
{
  using namespace ps;

  auto src = "(foo)";
  ParserRuntime runtime(src, State{});

  // between: run open parser, then main parser, then close parser, and return result from main parser

  Parser<std::string> p = between<std::string, std::string, std::string>(parseLit("("), parseLit("foo"), parseLit(")"));
  ParserResult<std::string> r = parseWithRuntime<std::string>(runtime, p);

  QVERIFY(isRight(r));
  QVERIFY(getParseSucceeded(r).parsed == "foo");
}

void FreeParsecTest::countTest()
{
  using namespace ps;

  auto src = "12345";
  ParserRuntime runtime(src, State{});

  // count: run parser n times and return list of results

  Parser<Many<Char>> p = count<Char>(3, digit);
  ParserResult<Many<Char>> r = parseWithRuntime<Many<Char>>(runtime, p);

  QVERIFY(isRight(r));
  Many<Char> parsed = getParseSucceeded(r).parsed;
  QVERIFY(parsed.size() == 3);
  QVERIFY(parsed.front() == '1');
  parsed.pop_front();
  QVERIFY(parsed.front() == '2');
  parsed.pop_front();
  QVERIFY(parsed.front() == '3');
}

void FreeParsecTest::discardTest()
{
  using namespace ps;

  auto src = "12345";
  ParserRuntime runtime(src, State{});

  // discard: run parser and discard its result, returning unit

  Parser<Unit> p = discard<Char>(digit);
  ParserResult<Unit> r = parseWithRuntime<Unit>(runtime, p);

  QVERIFY(isRight(r));
}

void FreeParsecTest::bothTest()
{
  using namespace ps;

  auto src = "1a345";
  ParserRuntime runtime(src, State{});

  // both: run two parsers and return pair of their results

  Parser<std::pair<Char, Char>> p = both<Char, Char>(digit, alpha);
  ParserResult<std::pair<Char, Char>> r = parseWithRuntime<std::pair<Char, Char>>(runtime, p);

  QVERIFY(isRight(r));
  std::pair<Char, Char> parsed = getParseSucceeded(r).parsed;
  QVERIFY(parsed.first == '1');
  QVERIFY(parsed.second == 'a');
}
