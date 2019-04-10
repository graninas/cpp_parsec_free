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

//    void singleDigitParserTest();
//    void digitParserTest();
//    void lowerCaseCharParserTest();
//    void upperCaseCharParserTest();
//    void symbolParserTest();
//    void parseFailureTest();
    void tryPTest();
//    void stateTest();

//    void bindPureTest();
//    void sequencedParsersTest();

//    void altCombinatorTest();
    void internalParsersTest();
//    void manyCombinatorTest();
};

PSTest::PSTest()
{
}

template <typename A>
void printError(const ps::ParseResult<A>& pr)
{
    if (isLeft(pr))
    {
        auto err = ps::getError(pr);
        std::cout << "\nError:\n" << err.message << "\n";
    }
}

//void PSTest::singleDigitParserTest()
//{
//    using namespace ps;

//    ParseResult<Char> result = parseP<Char>(digit, "1");

//    printError(result);

//    QVERIFY(isRight(result));
//    Char r = getParsed<Char>(result);
//    QVERIFY(r == '1');
//}

//void PSTest::digitParserTest()
//{
//    using namespace ps;

//    ParseResult<Char> result = parseP<Char>(digit, "1abc");

//    QVERIFY(isRight(result));
//    Char r = getParsed<Char>(result);
//    QVERIFY(r == '1');
//}

//void PSTest::lowerCaseCharParserTest()
//{
//    using namespace ps;

//    ParseResult<Char> result = parseP<Char>(lower, "abc");

//    QVERIFY(isRight(result));
//    Char r = getParsed<Char>(result);
//    QVERIFY(r == 'a');
//}

//void PSTest::upperCaseCharParserTest()
//{
//    using namespace ps;

//    ParseResult<Char> result = parseP<Char>(upper, "BCD");

//    QVERIFY(isRight(result));
//    Char r = getParsed<Char>(result);
//    QVERIFY(r == 'B');
//}

//void PSTest::symbolParserTest()
//{
//    using namespace ps;

//    ParseResult<Char> result = parseP<Char>(symbol('B'), "BCD");

//    QVERIFY(isRight(result));
//    Char r = getParsed<Char>(result);
//    QVERIFY(r == 'B');
//}

//void PSTest::parseFailureTest()
//{
//    using namespace ps;

//    ParseResult<Char> result = parseP<Char>(digit, "abc");

//    QVERIFY(isLeft(result));
//    QVERIFY(std::get<ParseError>(result).message == "Failed to parse digit: not a digit.");
//}

void PSTest::tryPTest()
{
    using namespace ps;

    std::function<
            ParserT<ParseResult<Char>>(
                ParseResult<Char>)
            > f = [](const ParseResult<Char>&) {
                return pure<ParseResult<Char>>(ParseSuccess<Char> { '1' } ); };

    auto pt = tryP(digitPL);
    auto pt2 = bind<ParseResult<Char>, ParseResult<Char>>(pt, f);

    auto result = parse(pt2, "abc");

    QVERIFY(isRight(result));
    QVERIFY(getParsed(result) == '1');
}

//void PSTest::stateTest()
//{
////    using namespace ps;

////    auto p = bind<Char, R>(getSt
////                digit,         [=](Char dg0) { return
////             bind<Char, R>(lower,         [=](Char ch1) { return
////             bind<Char, R>(symbol('2'),   [=](Char ch2) { return
////             pure<R>(R{dg0, ch1, ch2});
////            }); }); });

////    QVERIFY(isLeft(result));
////    QVERIFY(std::get<ParseError>(result).message == "Failed to parse digit: not a digit.");
//}

//struct R
//{
//    ps::Char dg0;
//    ps::Char ch1;
//    ps::Char ch2;
//};

//void PSTest::bindPureTest()
//{
//    using namespace ps;

//    auto p = bind<Char, R>(digit,       [=](Char d1) { return
//             pure<R>(R{d1, 'a', '0'});
//             });

//    ParseResult<R> result = parse(p, "1b2");

//    QVERIFY(isRight(result));
//    R r = getParsed<R>(result);
//    QVERIFY(r.dg0 == '1');
//    QVERIFY(r.ch1 == 'a');
//    QVERIFY(r.ch2 == '0');
//}

//void PSTest::sequencedParsersTest()
//{
//    using namespace ps;

//    auto p = bind<Char, R>(digit,         [=](Char dg0) { return
//             bind<Char, R>(lower,         [=](Char ch1) { return
//             bind<Char, R>(symbol('2'),   [=](Char ch2) { return
//             pure<R>(R{dg0, ch1, ch2});
//            }); }); });

//    ParseResult<R> result = parse(p, "1b2");

//    QVERIFY(isRight(result));
//    R r = getParsed<R>(result);
//    QVERIFY(r.dg0 == '1');
//    QVERIFY(r.ch1 == 'b');
//    QVERIFY(r.ch2 == '2');
//}

//void PSTest::altCombinatorTest()
//{
//    using namespace ps;

//    ParseResult<Char> result1 = parse(alt(upperPL, lowerPL), "A");
//    ParseResult<Char> result2 = parse(alt(upperPL, lowerPL), "a");

//    QVERIFY(isRight(result1));
//    QVERIFY(isRight(result2));
//    QVERIFY(getParsed(result1) == 'A');
//    QVERIFY(getParsed(result2) == 'a');
//}

void PSTest::internalParsersTest()
{
    using namespace ps;

    ParserL<Char> p1 =
       bindPL<Char, Char>(upperPL, [=](auto) { return
       bindPL<Char, Char>(lowerPL, [=](auto) { return lowerPL; }); });

    ParserL<Char> p2 =
       bindPL<Char, Char>(upperPL, [=](auto) { return
       bindPL<Char, Char>(upperPL, [=](auto) { return upperPL; }); });

    ParserT<Char> p =
       bind<Char, Char>(digit,
                        [=](auto) { return alt(p1, p2); });

    ParseResult<Char> result = parse<Char>(p, "1AAA2");

    printError(result);

    QVERIFY(isRight(result));
    Char ch = getParsed<Char>(result);
    QVERIFY(ch == 'A');
}


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

