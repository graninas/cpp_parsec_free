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
    void symbolParserTest();
    void parseFailureTest();

    void bindPureTest();
//    void sequencedParsersTest();

//    void altCombinatorTest();
//    void manyCombinatorTest();
};

PSTest::PSTest()
{
}

void PSTest::singleDigitParserTest()
{
    using namespace ps;

    ParseResult<Char> result = parse<Char>(digit, "1");

    QVERIFY(isRight(result));
    Char r = getParsed<Char>(result);
    QVERIFY(r == '1');
}

void PSTest::digitParserTest()
{
    using namespace ps;

    ParseResult<Char> result = parse<Char>(digit, "1abc");

    QVERIFY(isRight(result));
    Char r = getParsed<Char>(result);
    QVERIFY(r == '1');
}

void PSTest::lowerCaseCharParserTest()
{
    using namespace ps;

    ParseResult<Char> result = parse<Char>(lower, "abc");

    QVERIFY(isRight(result));
    Char r = getParsed<Char>(result);
    QVERIFY(r == 'a');
}

void PSTest::upperCaseCharParserTest()
{
    using namespace ps;

    ParseResult<Char> result = parse<Char>(upper, "BCD");

    QVERIFY(isRight(result));
    Char r = getParsed<Char>(result);
    QVERIFY(r == 'B');
}

void PSTest::symbolParserTest()
{
    using namespace ps;

    ParseResult<Char> result = parse<Char>(symbol('B'), "BCD");

    QVERIFY(isRight(result));
    Char r = getParsed<Char>(result);
    QVERIFY(r == 'B');
}

void PSTest::parseFailureTest()
{
    using namespace ps;

    ParseResult<Char> result = parse<Char>(digit, "abc");

    QVERIFY(isLeft(result));
    QVERIFY(std::get<ParseError>(result).message == "Failed to parse digit: not a digit.");
}

struct R
{
    ps::Char dg0;
    ps::Char ch1;
    ps::Char ch2;
};

void PSTest::bindPureTest()
{
//    using namespace ps;

//    ParserL<R> p = ps::bind<Char, R>(digit,       [=](Char d1) { return
//                   ps::pure<R>(R{d1, 'a', '0'}); });

//    ParseResult<R> result = parse(p, "1b2");

//    QVERIFY(isRight(result));
//    R r = getParsed<R>(result);
//    QVERIFY(r.dg0 == '1');
//    QVERIFY(r.ch1 == 'a');
//    QVERIFY(r.ch2 == '0');
}

//void PSTest::sequencedParsersTest()
//{
//    using namespace ps;

//    ParserL<R> p = ps::bind<Char, R>(digit,         [=](Char dg0) { return
//                   ps::bind<Char, R>(lower,         [=](Char ch1) { return
//                   ps::bind<Char, R>(symbol('2'),   [=](Char ch2) { return
//                   ps::pure<R>(R{dg0, ch1, ch2}); }); }); });

//    ParseResult<R> result = parse(p, "1b2");

//    QVERIFY(isRight(result));
//    R r = getParsed<R>(result);
//    QVERIFY(r.dg0 == '1');
//    QVERIFY(r.ch1 == 'b');
//    QVERIFY(r.ch2 == '2');
//}

//void PSTest::altCombinatorTest()
//{
////    using namespace ps;

////    ParseResult<Char> result1 = parse(alt(upper, lower), "A");
////    ParseResult<Char> result2 = parse(alt(upper, lower), "a");

////    QVERIFY(isRight(result1));
////    QVERIFY(isRight(result2));
////    QVERIFY(getParsed(result1) == 'A');
////    QVERIFY(getParsed(result2) == 'a');
//}

//struct R2
//{
//    ps::Many<ps::Char> ds;
//    ps::Char ch;
//};

//void PSTest::manyCombinatorTest()
//{
//    using namespace ps;

//    const std::string s = "1234b2";

////    ParserL<R> p = ps::bind<Many<Digit>, R2>(many(digit),   [=](const Many<Digit>& ds) { return
////                   ps::bind<Char,        R2>(lowerCaseChar, [=](Char ch1)                     { return
////                   ps::pure<R2>(R2{ds, ch1}); }); });

////    ParseResult<R> result = parse(p, s);

////    QVERIFY(isRight(result));
////    QVERIFY(std::get<R>(result).ch == 'b');
////    QVERIFY(std::get<R>(result).d == 1);
//}

QTEST_APPLESS_MAIN(PSTest)

#include "tst_parsec.moc"

