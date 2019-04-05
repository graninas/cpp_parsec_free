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
//    void digitParserTest();
//    void lowerCaseCharParserTest();
//    void upperCaseCharParserTest();
//    void parseFailureTest();

//    void bindPureTest();
//    void sequencedParsersTest();

    void manyCombinatorTest();
};

PSTest::PSTest()
{
}

void PSTest::singleDigitParserTest()
{
    using namespace ps;

    const std::string s = "1";
    ParseResult<Digit> result = parse<Digit>(parseDigit(), s);

    ParseError e = getError(result);

    std::cout << e.message;

    QVERIFY(isRight(result));

    Digit r = getParsed<Digit>(result);
    QVERIFY(r == 1);
}

//void PSTest::digitParserTest()
//{
//    using namespace ps;

//    const std::string s = "1abc";
//    ParseResult<Digit> result = parse<Digit>(parseDigit(), s);

//    QVERIFY(isRight(result));

//    Digit r = getParsed<Digit>(result);
//    QVERIFY(r == 1);
//}

//void PSTest::lowerCaseCharParserTest()
//{
//    using namespace ps;

//    const std::string s = "abc";
//    ParseResult<Char> result = parse<Char>(lowerCaseChar, s);

//    QVERIFY(isRight(result));

//    Char r = getParsed<Char>(result);
//    QVERIFY(r == 'a');
//}

//void PSTest::upperCaseCharParserTest()
//{
//    using namespace ps;

//    const std::string s = "BCD";
//    ParseResult<Char> result = parse<Char>(upperCaseChar, s);

//    QVERIFY(isRight(result));

//    Char r = getParsed<Char>(result);
//    QVERIFY(r == 'B');
//}

//void PSTest::parseFailureTest()
//{
//    using namespace ps;

//    const std::string s = "abc";
//    ParseResult<Digit> result = parse<Digit>(parseDigit(), s);

//    QVERIFY(isLeft(result));
//    std::cout << std::get<ParseError>(result).message;
//    QVERIFY(std::get<ParseError>(result).message == "Failed to parse digit: not a digit.");
//}

//struct R
//{
//    ps::Digit d;
//    ps::Char ch;
//};

//void PSTest::bindPureTest()
//{
//    using namespace ps;

//    const std::string s = "1b2";

//    ParserL<R> p = ps::bind<Digit, R>(digit,       [=](Digit d1) { return
//                   ps::pure<R>(R{d1, 'a'}); });

//    ParseResult<R> result = parse(p, s);

//    QVERIFY(isRight(result));
//    R r = getParsed<R>(result);
//    QVERIFY(r.ch == 'a');
//    QVERIFY(r.d == 1);
//}

//void PSTest::sequencedParsersTest()
//{
//    using namespace ps;

//    const std::string s = "1b2";

//    ParserL<R> p = ps::bind<Digit, R>(digit,         [=](Digit d1) { return
//                   ps::bind<Char,  R>(lowerCaseChar, [=](Char ch1) { return
//                   ps::pure<R>(R{d1, ch1}); }); });

//    ParseResult<R> result = parse(p, s);

//    QVERIFY(isRight(result));
//    R r = getParsed<R>(result);
//    QVERIFY(r.ch == 'b');
//    QVERIFY(r.d == 1);
//}

//struct R2
//{
//    ps::Many<ps::Digit> ds;
//    ps::Char ch;
//};

void PSTest::manyCombinatorTest()
{
//    using namespace ps;

//    const std::string s = "1234b2";

//    ParserL<R> p = ps::bind<Many<Digit>, R2>(many(digit),   [=](const Many<Digit>& ds) { return
//                   ps::bind<Char,        R2>(lowerCaseChar, [=](Char ch1)                     { return
//                   ps::pure<R2>(R2{ds, ch1}); }); });

//    ParseResult<R> result = parse(p, s);

//    QVERIFY(isRight(result));
//    QVERIFY(std::get<R>(result).ch == 'b');
//    QVERIFY(std::get<R>(result).d == 1);
}

QTEST_APPLESS_MAIN(PSTest)

#include "tst_parsec.moc"

