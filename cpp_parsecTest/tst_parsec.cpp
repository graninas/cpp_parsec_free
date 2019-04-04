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
    void digitParserTest();
    void lowerCaseCharParserTest();
    void upperCaseCharParserTest();
    void parseFailureTest();

    void bindPureTest();
    void sequenceCombinatorTest();
};

PSTest::PSTest()
{
}

void PSTest::singleDigitParserTest()
{
    using namespace ps;

    const std::string s = "1";
    ParseResult<Digit> result = parse<Digit>(parseDigit(), s);

    QVERIFY(isRight(result));

    Digit r = std::get<Digit>(result);
    QVERIFY(r == 1);
}

void PSTest::digitParserTest()
{
    using namespace ps;

    const std::string s = "1abc";
    ParseResult<Digit> result = parse<Digit>(parseDigit(), s);

    QVERIFY(isRight(result));

    Digit r = std::get<Digit>(result);
    QVERIFY(r == 1);
}

void PSTest::lowerCaseCharParserTest()
{
    using namespace ps;

    const std::string s = "abc";
    ParseResult<Char> result = parse<Char>(lowerCaseChar, s);

    QVERIFY(isRight(result));

    Char r = std::get<Char>(result);
    QVERIFY(r == 'a');
}

void PSTest::upperCaseCharParserTest()
{
    using namespace ps;

    const std::string s = "BCD";
    ParseResult<Char> result = parse<Char>(upperCaseChar, s);

    QVERIFY(isRight(result));

    Char r = std::get<Char>(result);
    QVERIFY(r == 'B');
}

void PSTest::parseFailureTest()
{
    using namespace ps;

    const std::string s = "abc";
    ParseResult<Digit> result = parse<Digit>(parseDigit(), s);

    QVERIFY(isLeft(result));
    std::cout << std::get<ParseError>(result).message;
    QVERIFY(std::get<ParseError>(result).message == "Failed to parse digit: not a digit.");
}

struct R
{
    ps::Digit d;
    ps::Char ch;
};

void PSTest::bindPureTest()
{
    using namespace ps;

    const std::string s = "1a2";

    ParserL<R> p = ps::bind<Digit, R>(digit,       [=](Digit d1) { return
                   ps::pure<R>(R{d1, 'a'}); });

    ParseResult<R> result = parse(p, s);

    QVERIFY(isRight(result));
    QVERIFY(std::get<R>(result).ch == 'a');
    QVERIFY(std::get<R>(result).d == 1);
}

void PSTest::sequenceCombinatorTest()
{
    using namespace ps;

    const std::string s = "1a2";

    std::function<ParserL<Digit>(Digit)> f
            = [=](Digit d1) { return ps::pure<Digit>(d1); };

    ParserL<Digit> p = ps::bind<Digit, Digit>(ps::pure<Digit>(1), f);

    ParseResult<Digit> result = parse(p, s);

    QVERIFY(isRight(result));
}

QTEST_APPLESS_MAIN(PSTest)

#include "tst_parsec.moc"

