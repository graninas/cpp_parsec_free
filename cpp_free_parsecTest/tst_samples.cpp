#include <QString>
#include <QtTest>

#include <variant>
#include <iostream>

#include "common.h"

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

ps::Parser<std::string> capitalizedWord()
{
  using namespace ps;

  Parser<char> firstChar = upper;
  Parser<std::list<char>> restChars = many(lower);

  auto seqp = sequence(firstChar, restChars);

  return merge(seqp);
}

ps::Parser<std::string> firstNameParser()
{
  return capitalizedWord();
}

ps::Parser<std::string> lastNameParser()
{
  return capitalizedWord();
}

ps::Parser<int> ageParser()
{
  return ps::mergeTo<int>(ps::many(ps::digit));
}

ps::Parser<std::string> ssnParser()
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
      digit, digit, digit, digit);
  return mergeTo<std::string>(parser);
}

ps::Parser<PersonInfo> personInfoParser()
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

// Define the Abstract Syntax Tree (AST) for the custom language
struct ASTNode
{
  enum class Type
  {
    Parameter,
    Operator,
    Number
  };

  Type type;
  std::string value; // For parameters and operators
  int number;        // For numeric values

  std::shared_ptr<ASTNode> left;  // Left child (for binary operators)
  std::shared_ptr<ASTNode> right; // Right child (for binary operators)

  ASTNode(Type t, const std::string &val) : type(t), value(val), number(0) {}
  ASTNode(Type t, int num) : type(t), number(num) {}
};

ps::Parser<std::string> parameterIdentifier()
{
  using namespace ps;
  Parser<char> firstChar = alt(upper, lower);
  Parser<std::list<char>> restChars = many(alphanum);
  auto seqp = sequence(firstChar, restChars);
  return merge(seqp);
}

// Parser for the custom language

ps::Parser<std::shared_ptr<ASTNode>> expressionParser();

ps::Parser<std::shared_ptr<ASTNode>> parensParser()
{
  using namespace ps;

  Parser<std::shared_ptr<ASTNode>> parser =
      between(
          parseChar('('),
          expressionParser(),
          parseChar(')')) +
      "parens";

  return parser;
}

ps::Parser<std::shared_ptr<ASTNode>> parameterParser()
{
  using namespace ps;
  return fmap<std::string, std::shared_ptr<ASTNode>>(
      [](const std::string &param)
      {
        return std::make_shared<ASTNode>(ASTNode::Type::Parameter, param);
      },
      between(parseChar('['), parameterIdentifier(), parseChar(']')))
      + "param";
}

ps::Parser<std::shared_ptr<ASTNode>> numberParser()
{
  using namespace ps;
  return fmap<int, std::shared_ptr<ASTNode>>(
      [](int num)
      {
        return std::make_shared<ASTNode>(ASTNode::Type::Number, num);
      },
      mergeTo<int>(many1(digit)))
      + "number";
}

ps::Parser<std::shared_ptr<ASTNode>> termParser()
{
  using namespace ps;
  return alt(try_(numberParser()),
             alt(try_(parameterParser()), parensParser()));
}

ps::Parser<std::shared_ptr<ASTNode>> operatorParser()
{
  using namespace ps;
  return fmap<std::string, std::shared_ptr<ASTNode>>(
      [](const std::string &op)
      {
        return std::make_shared<ASTNode>(ASTNode::Type::Operator, op);
      },
      choice(parseLit(">=") + "ge",
            parseLit("+") + "add",
            parseLit("-") + "sub",
            parseLit("*") + "mul",
            parseLit("/") + "div"))
      + "op";
}

ps::Parser<std::shared_ptr<ASTNode>> operatorExprParser()
{
  using namespace ps;

  auto seqp = sequence(
      skip(parseChar('(')),
      expressionParser(),
      operatorParser(),
      expressionParser(),
      skip(parseChar(')')))
      + "operator expr";

  Parser<std::shared_ptr<ASTNode>> mappedSeqp = fmap<
      std::tuple<std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>>,
      std::shared_ptr<ASTNode>>(
      [](const std::tuple<std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>> &t)
      {
        auto left = std::get<0>(t);
        auto op = std::get<1>(t);
        auto right = std::get<2>(t);

        // Set the left and right children of the operator node
        op->left = left;
        op->right = right;

        return op;
      },
      seqp);
  return mappedSeqp;
}



ps::Parser<std::shared_ptr<ASTNode>> expressionParser()
{
  using namespace ps;

  // Define the parser lazily to handle recursion
  return lazy<std::shared_ptr<ASTNode>>(
    [](){
      return
          alt(
            try_(operatorExprParser()),
            termParser()
          );
    });
}

// Evaluate the AST using the parameter map
int evaluateAST(const std::shared_ptr<ASTNode> &node, const std::map<std::string, int> &params)
{
  if (!node)
  {
    throw std::runtime_error("Invalid AST node");
  }

  switch (node->type)
  {
  case ASTNode::Type::Parameter:
  {
    auto it = params.find(node->value);
    if (it == params.end())
    {
      throw std::runtime_error("Undefined parameter: " + node->value);
    }
    return it->second;
  }
  case ASTNode::Type::Number:
    return node->number;
  case ASTNode::Type::Operator:
  {
    int leftValue = evaluateAST(node->left, params);
    int rightValue = evaluateAST(node->right, params);

    if (node->value == ">=")
    {
      return leftValue >= rightValue;
    }
    else if (node->value == "+")
    {
      return leftValue + rightValue;
    }
    else if (node->value == "-")
    {
      return leftValue - rightValue;
    }
    else if (node->value == "*")
    {
      return leftValue * rightValue;
    }
    else if (node->value == "/")
    {
      if (rightValue == 0)
      {
        throw std::runtime_error("Division by zero");
      }
      return leftValue / rightValue;
    }
    else
    {
      throw std::runtime_error("Unknown operator: " + node->value);
    }
  }
  default:
    throw std::runtime_error("Unknown AST node type");
  }
}

void SamplesTest::personInfoParserTest()
{
  using namespace ps;

  std::string src = "John,Doe,30,123-45  -    6789}";
  ParserRuntime runtime(src, State{});

  Parser<PersonInfo> p = personInfoParser();

  ParserResult<PersonInfo> r = parseWithRuntime<PersonInfo>(runtime, p);

  QVERIFY(isRight(r));
  PersonInfo info = getParseSucceeded(r).parsed;
  QVERIFY(info.firstName == "John");
  QVERIFY(info.lastName == "Doe");
  QVERIFY(info.age == 30);
  QVERIFY(info.ssn == "123-45-6789");
}

void SamplesTest::numberTest()
{
  using namespace ps;

  std::string src = "2";
  ParserRuntime runtime(src, State{});

  Parser<std::shared_ptr<ASTNode>> parser = numberParser();

  ParserResult<std::shared_ptr<ASTNode>> result = parseWithRuntime(runtime, parser);

  QVERIFY(isRight(result));

  // Evaluate the AST
  std::map<std::string, int> params;
  std::shared_ptr<ASTNode> ast = getParseSucceeded(result).parsed;
  int evaluationResult = evaluateAST(ast, params);

  // Verify the result
  QVERIFY(evaluationResult == 2);
}

void SamplesTest::numberExprTest()
{
  using namespace ps;

  std::string src = "2";
  ParserRuntime runtime(src, State{});

  Parser<std::shared_ptr<ASTNode>> parser = expressionParser();

  ParserResult<std::shared_ptr<ASTNode>> result = parseWithRuntime(runtime, parser);

  // printMessages(runtime);

  QVERIFY(isRight(result));

  // Evaluate the AST
  std::map<std::string, int> params;
  std::shared_ptr<ASTNode> ast = getParseSucceeded(result).parsed;
  int evaluationResult = evaluateAST(ast, params);

  // Verify the result
  QVERIFY(evaluationResult == 2);
}

void SamplesTest::numberExprInParensTest()
{
  using namespace ps;

  std::string src = "(2)";
  ParserRuntime runtime(src, State{});

  Parser<std::shared_ptr<ASTNode>> parser = expressionParser();

  ParserResult<std::shared_ptr<ASTNode>> result = parseWithRuntime(runtime, parser);

  // printMessages(runtime);

  QVERIFY(isRight(result));

  // Evaluate the AST
  std::map<std::string, int> params;
  std::shared_ptr<ASTNode> ast = getParseSucceeded(result).parsed;
  int evaluationResult = evaluateAST(ast, params);

  // Verify the result
  QVERIFY(evaluationResult == 2);
}


void SamplesTest::simpleParenExprTest()
{
  using namespace ps;

  std::string src = "((2)+(3))";
  ParserRuntime runtime(src, State{});

  Parser<std::shared_ptr<ASTNode>> parser = expressionParser();

  ParserResult<std::shared_ptr<ASTNode>> result = parseWithRuntime(runtime, parser);

  // printMessages(runtime);

  QVERIFY(isRight(result));

  // Evaluate the AST
  std::map<std::string, int> params;
  std::shared_ptr<ASTNode> ast = getParseSucceeded(result).parsed;
  int evaluationResult = evaluateAST(ast, params);

  // Verify the result
  QVERIFY(evaluationResult == 5);
}

void SamplesTest::simpleExprTest()
{
  using namespace ps;

  std::string src = "(2+3)";
  ParserRuntime runtime(src, State{});

  Parser<std::shared_ptr<ASTNode>> parser = expressionParser();

  ParserResult<std::shared_ptr<ASTNode>> result = parseWithRuntime(runtime, parser);

  // printMessages(runtime);

  QVERIFY(isRight(result));

  // Evaluate the AST
  std::map<std::string, int> params;
  std::shared_ptr<ASTNode> ast = getParseSucceeded(result).parsed;
  int evaluationResult = evaluateAST(ast, params);

  // Verify the result
  QVERIFY(evaluationResult == 5);
}

void SamplesTest::customLanguageParserTest()
{
  using namespace ps;

  // Expressions example: `([P1]-(([P2]+1)*([P3]/[P4])))`
  // Where P1, P2, P3, P4 are parameter names, and the operators are >=, +, *, /, and parentheses.
  // Actual parameters are defined separately as a map.
  // When evaluating, parameter names will be replaced by their values from the map, and the expression will be evaluated according to operator precedence.

  // Example parameter map
  std::map<std::string, int> params = {
      {"P1", 5},
      {"P2", 10},
      {"P3", 30},
      {"P4", 2}};

  // Parse the expression
  //                         -160    11   165   15
  std::string expression = "([P1]-(([P2]+1)*([P3]/[P4])))";
  ParserRuntime runtime(expression, State{});
  Parser<std::shared_ptr<ASTNode>> parser = expressionParser();

  ParserResult<std::shared_ptr<ASTNode>> result = parseWithRuntime(runtime, parser);

  // printMessages(runtime);

  QVERIFY(isRight(result));

  // Evaluate the AST
  std::shared_ptr<ASTNode> ast = getParseSucceeded(result).parsed;
  int evaluationResult = evaluateAST(ast, params);

  // Verify the result
  QVERIFY(evaluationResult == (-160));
}
