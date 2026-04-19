#include <QString>
#include <QtTest>

#include <variant>
#include <iostream>

#include "ps/ps.h"
#include "ps/core/parser/merge.h"

#include "tst_samples.h"
#include <QTest>

SamplesTest::SamplesTest(QObject *parent) : QObject(parent) {}

struct PersonInfo
{
    std::string firstName;
    std::string lastName;
    int age;
    std::string ssn;
};

ps::ParserL<std::string> capitalizedWord()
{
  using namespace ps;

  ParserL<char> firstChar = upper;
  ParserL<std::list<char>> restChars = many(lower);

  auto seqp = sequence(firstChar, restChars);

  return merge(seqp);
}

ps::ParserL<std::string> firstNameParser()
{
  return capitalizedWord();
}

ps::ParserL<std::string> lastNameParser()
{
  return capitalizedWord();
}

ps::ParserL<int> ageParser()
{
    return ps::mergeTo<int>(ps::many(ps::digit));
}


ps::ParserL<std::string> ssnParser()
{
    using namespace ps;
    // Parse 3 digits, dash, 2 digits, dash, 4 digits
    auto parser = sequence(
        digit, digit, digit,
        skip(many(space)),
        parseLit("-"),
        skip(many(space)),
        digit, digit,
        skip(many(space)),
        parseLit("-"),
        skip(many(space)),
        digit, digit, digit, digit
    );
    return mergeTo<std::string>(parser);
}

ps::ParserL<PersonInfo> personInfoParser()
{
  using namespace ps;

  // Build a sequence of parsers. skip(comma) returns Unit and will be removed
  // from the resulting tuple by the sequence(...) machinery.
  auto seqp = sequence(
      firstNameParser(),
      skip(comma),
      lastNameParser(),
      skip(comma),
      ageParser(),
      skip(comma),
      ssnParser());

  // Map the resulting tuple into the PersonInfo aggregate using as<>
  return as<PersonInfo>(seqp);
}

void SamplesTest::personInfoParserTest()
{
    using namespace ps;

    auto src = "John,Doe,30,123-45  -    6789}";
    ParserRuntime runtime(src, State{});

    ParserL<PersonInfo> p = personInfoParser();

    ParserResult<PersonInfo> r = parseWithRuntime<PersonInfo>(runtime, p);

    QVERIFY(isRight(r));
    PersonInfo info = getParseSucceeded(r).parsed;
    QVERIFY(info.firstName == "John");
    QVERIFY(info.lastName == "Doe");
    QVERIFY(info.age == 30);
    QVERIFY(info.ssn == "123-45-6789");
}
