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
    void manyCombinatorTest();
    void parseFailureTest();
    void tryThrowParserLTest();
    void tryThrowParserTTest();
    void safeVSTryPTest();

    void bindPureTest();
    void applicativeTest();
    void sequencedParsersTest();
    void forgetCombinatorsTest();

    void alt1Test();
    void alt2Test();
    void internalParsersTest();

    void employeeTest();
};

PSTest::PSTest()
{
}

template <typename A>
void printError(const ps::ParserResult<A>& pr)
{
    if (isLeft(pr))
    {
        auto err = ps::getError(pr);
        std::cout << "\nError:\n" << err.message << "\n";
    }
}

void PSTest::singleDigitParserTest()
{
    using namespace ps;

    ParserResult<Char> result = parse<Char>(digit, "1");

    printError(result);

    QVERIFY(isRight(result));
    Char r = getParsed<Char>(result);
    QVERIFY(r == '1');
}

void PSTest::digitParserTest()
{
    using namespace ps;

    ParserResult<Char> result = parse<Char>(digit, "1abc");

    QVERIFY(isRight(result));
    Char r = getParsed<Char>(result);
    QVERIFY(r == '1');
}

void PSTest::lowerCaseCharParserTest()
{
    using namespace ps;

    ParserResult<Char> result = parse<Char>(lower, "abc");

    QVERIFY(isRight(result));
    Char r = getParsed<Char>(result);
    QVERIFY(r == 'a');
}

void PSTest::upperCaseCharParserTest()
{
    using namespace ps;

    ParserResult<Char> result = parse<Char>(upper, "BCD");

    QVERIFY(isRight(result));
    Char r = getParsed<Char>(result);
    QVERIFY(r == 'B');
}

void PSTest::symbolParserTest()
{
    using namespace ps;

    ParserResult<Char> result = parse<Char>(symbol('B'), "BCD");

    QVERIFY(isRight(result));
    Char r = getParsed<Char>(result);
    QVERIFY(r == 'B');
}

struct R2
{
    ps::Many<ps::Char> ds;
    ps::Char ch;
};

void PSTest::manyCombinatorTest()
{
    using namespace ps;

    ParserT<Many<Char>> p = manyPL<Char>(digitThrowPL);

    ParserResult<Many<Char>> result = parse(p, "4321");

    QVERIFY(isRight(result));
    Many<Char> parsed = getParsed(result);
    QVERIFY(parsed.size() == 4);
    QVERIFY(parsed.front() == '4');
    parsed.pop_front();
    QVERIFY(parsed.front() == '3');
    parsed.pop_front();
    QVERIFY(parsed.front() == '2');
    parsed.pop_front();
    QVERIFY(parsed.front() == '1');
}

void PSTest::parseFailureTest()
{
    using namespace ps;

    ParserResult<Char> result = parse<Char>(digit, "abc");

    QVERIFY(isLeft(result));
    QVERIFY(std::get<ParserFailed>(result).message == "Failed to parse digit: not a digit.");
}

void PSTest::tryThrowParserLTest()
{
    using namespace ps;

    auto f = [](const ParserResult<Char>&)
        {
            return pure<ParserResult<Char>>(ParserSucceeded<Char> { '1' } );
        };

    ParserT<ParserResult<Char>> pt = tryP(digitThrowPL);
    auto pt2 = bind<ParserResult<Char>, ParserResult<Char>>(pt, f);

    auto result = parse(pt2, "abc");

    QVERIFY(isLeft(result));
}

void PSTest::tryThrowParserTTest()
{
// This does not work.
//    using namespace ps;

//    auto f = [](const ParserResult<Char>&)
//        {
//            return pure<ParserResult<Char>>(ParserSucceeded<Char> { '1' } );
//        };

//    ParserT<ParserResult<Char>> pt = tryP(digit);
//    auto pt2 = bind<ParserResult<Char>, ParserResult<Char>>(pt, f);

//    auto result = parse(pt2, "abc");

//    QVERIFY(isRight(result));
//    QVERIFY(getParsed(result) == '1');
}

/*

-- f should fail with err1
f = do
    try (parserA >> parserB >> fail err1)
        >>= \eRes -> fail err2

-- g should fail with err2
g = do
    safe (parserA >> parserB >> fail err1)
        >>= \eRes -> fail err2

*/

void PSTest::safeVSTryPTest()
{
    using namespace ps;

    auto f = [](ParserResult<Unit>) {
        throw std::runtime_error("err2");
        return pure(unit);
    };

    ParserL<Unit> internalP =
            bind<Char, Unit>(digitThrowPL, [](Char) { return
            bind<Char, Unit>(digitThrowPL, [](Char) {
                throw std::runtime_error("err1");
                return purePL(unit);
        });
    });

    ParserT<Unit> triedP = bind<ParserResult<Unit>, Unit>(tryP(internalP), f);
    ParserT<Unit> safedP = bind<ParserResult<Unit>, Unit>(safeP(internalP), f);

    auto result1 = parse(triedP, "123");
    auto result2 = parse(safedP, "123");

    QVERIFY(isLeft(result1));
    QVERIFY(getError(result1).message == "err1");
    QVERIFY(isLeft(result2));
    QVERIFY(getError(result2).message == "err2");
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

    auto p = bind<Char, R>(digit,       [=](Char d1) { return
             pure<R>(R{d1, 'a', '0'});
             });

    ParserResult<R> result = parse(p, "1b2");

    QVERIFY(isRight(result));
    R r = getParsed<R>(result);
    QVERIFY(r.dg0 == '1');
    QVERIFY(r.ch1 == 'a');
    QVERIFY(r.ch2 == '0');
}

void PSTest::applicativeTest()
{
    using namespace ps;

    auto mkR = [](Char dg0, Char ch1, Char ch2) { return R { dg0, ch1, ch2 }; };

    auto p = app<R, Char, Char, Char>(mkR, digit, lower, digit);

    ParserResult<R> result = parse(p, "1b2");

    QVERIFY(isRight(result));
    R r = getParsed<R>(result);
    QVERIFY(r.dg0 == '1');
    QVERIFY(r.ch1 == 'b');
    QVERIFY(r.ch2 == '2');
}

void PSTest::sequencedParsersTest()
{
    using namespace ps;

    auto p = bind<Char, R>(digit,         [=](Char dg0) { return
             bind<Char, R>(lower,         [=](Char ch1) { return
             bind<Char, R>(symbol('2'),   [=](Char ch2) { return
             pure<R>(R{dg0, ch1, ch2});
            }); }); });

    ParserResult<R> result = parse(p, "1b2");

    QVERIFY(isRight(result));
    R r = getParsed<R>(result);
    QVERIFY(r.dg0 == '1');
    QVERIFY(r.ch1 == 'b');
    QVERIFY(r.ch2 == '2');
}

void PSTest::forgetCombinatorsTest()
{
    using namespace ps;

    auto fst = forgetFirst(letter, digit);
    auto snd = forgetSecond(letter, digit);

    ParserResult<Char> result1 = parse(fst, "a1");
    ParserResult<Char> result2 = parse(snd, "a1");

    QVERIFY(isRight(result1));
    QVERIFY(isRight(result2));
    QVERIFY(getParsed<Char>(result1) == '1');
    QVERIFY(getParsed<Char>(result2) == 'a');
}

void PSTest::alt1Test()
{
    using namespace ps;

    ParserResult<Char> result1 = parse(alt(upperPL, lowerPL), "A");
    ParserResult<Char> result2 = parse(alt(upperPL, lowerPL), "a");

    QVERIFY(isRight(result1));
    QVERIFY(isRight(result2));
    QVERIFY(getParsed(result1) == 'A');
    QVERIFY(getParsed(result2) == 'a');
}

void PSTest::alt2Test()
{
    using namespace ps;

    ParserResult<Char> result1 = parse(alt(upperThrowPL, lowerThrowPL), "A");
    ParserResult<Char> result2 = parse(alt(upperThrowPL, lowerThrowPL), "a");

    QVERIFY(isRight(result1));
    QVERIFY(isRight(result2));
    QVERIFY(getParsed(result1) == 'A');
    QVERIFY(getParsed(result2) == 'a');
}

void PSTest::internalParsersTest()
{
    using namespace ps;

    ParserL<Char> p1 =
       bind<Char, Char>(upperThrowPL, [=](auto) { return
       bind<Char, Char>(lowerThrowPL, [=](auto) { return lowerThrowPL; }); });

    ParserL<Char> p2 =
       bind<Char, Char>(upperThrowPL, [=](auto) { return
       bind<Char, Char>(upperThrowPL, [=](auto) { return upperThrowPL; }); });

    ParserT<Char> p =
       bind<Char, Char>(digit,
                        [=](auto) { return alt(p1, p2); });

    ParserResult<Char> result = parse<Char>(p, "1AAA2");

    printError(result);

    QVERIFY(isRight(result));
    Char ch = getParsed<Char>(result);
    QVERIFY(ch == 'A');
}

struct Employee
{
    int age;
    std::string surname;
    std::string forename;
    double salary;
};

void PSTest::employeeTest()
{
    using namespace ps;

    auto mkEmployee = [](
        int age, const string& sn, const string& fn, double s) {
        return Employee {age, sn, fn, s};
    };

    auto prefix = lit("employee") >> between(spaces, symbol('{'));
    auto postfix = between(spaces, symbol('}'));
    auto comma = between(spaces, symbol(','));

    auto age = intP;
    auto firstName = quotedString;
    auto lastName = quotedString;
    auto salary = doubleP;

    ParserT<Employee> employeeParser =
        app<Employee, int, string, string, double>(
            mkEmployee,
            prefix >> intP,                 // age
            comma >> quotedString,          // first name
            comma >> quotedString,          // last name
            comma >> (doubleP << postfix)); // salary

    auto s = "employee {35, “Jane”, “Street”, 50000.0}";
    ParserResult<Employee> result = parse(employeeParser, s);

    if (isLeft(result)) {
        cout << "Parse error: " << getError(result);
    }
    else {
        Employee employee = getParsed(result);
    }
}


QTEST_APPLESS_MAIN(PSTest)

#include "tst_parsec.moc"

