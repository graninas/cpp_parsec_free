#include <QString>
#include <QtTest>

#include <variant>
#include <iostream>

#include "ps/ps.h"

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

ps::ParserL<std::string> firstNameParser()
{
  // For a slightly more realistic test, let's parse a first name that is 4 alphanumeric characters long.
  return ps::manyCharsToString(ps::count<char>(4, ps::alphanum));
}

ps::ParserL<std::string> lastNameParser()
{
  // Dummy for now, just to test the personInfoParser. We will implement it properly later.
  return ps::manyCharsToString(ps::count<char>(3, ps::alphanum));
}

ps::ParserL<int> ageParser()
{
  // Dummy for now, just to test the personInfoParser. We will implement it properly later.
  return ps::fmap<std::string, int>([](const std::string &s) { return std::stoi(s); }, ps::parseLit("30"));
}

ps::ParserL<std::string> ssnParser()
{
  // Dummy for now, just to test the personInfoParser. We will implement it properly later.
  return ps::parseLit("123-45-6789");
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

    auto src = "John,Doe,30,123-45-6789";
    ParserRuntime runtime(src, State{});

    ParserL<PersonInfo> p = personInfoParser();

    ParserResult<PersonInfo> r = parse_with_runtime<PersonInfo>(runtime, p);

    QVERIFY(isRight(r));
    PersonInfo info = getParseSucceeded(r).parsed;
    QVERIFY(info.firstName == "John");
    QVERIFY(info.lastName == "Doe");
    QVERIFY(info.age == 30);
    QVERIFY(info.ssn == "123-45-6789");
}
