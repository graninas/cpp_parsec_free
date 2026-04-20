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
        digit, digit, digit, digit
    );
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
struct ASTNode {
    enum class Type { Parameter, Operator, Number, Parentheses };

    Type type;
    std::string value; // For parameters and operators
    int number;        // For numeric values

    std::shared_ptr<ASTNode> left;  // Left child (for binary operators)
    std::shared_ptr<ASTNode> right; // Right child (for binary operators)

    ASTNode(Type t, const std::string& val) : type(t), value(val), number(0) {}
    ASTNode(Type t, int num) : type(t), number(num) {}
};

// Parser for the custom language
ps::Parser<std::shared_ptr<ASTNode>> parameterParser() {
    using namespace ps;
    return fmap<std::tuple<std::string, std::string, std::string>, std::shared_ptr<ASTNode>>(
        [](const std::tuple<std::string, std::string, std::string>& tuple) {
            return std::make_shared<ASTNode>(ASTNode::Type::Parameter, std::get<1>(tuple));
        },
        sequence(parseLit("["), capitalizedWord(), parseLit("]")));
}

ps::Parser<std::shared_ptr<ASTNode>> numberParser() {
    using namespace ps;
    return fmap<int, std::shared_ptr<ASTNode>>(
        [](int num) {
            return std::make_shared<ASTNode>(ASTNode::Type::Number, num);
        },
        mergeTo<int>(many(digit)));
}

ps::Parser<std::shared_ptr<ASTNode>> operatorParser() {
    using namespace ps;
    return fmap<std::string, std::shared_ptr<ASTNode>>(
        [](const std::string& op) {
            return std::make_shared<ASTNode>(ASTNode::Type::Operator, op);
        },
        choice(parseLit(">="), parseLit("+"), parseLit("*"), parseLit("/")));
}

ps::Parser<std::shared_ptr<ASTNode>> expressionParser();

ps::Parser<std::shared_ptr<ASTNode>> parenthesesParser() {
    using namespace ps;
    return fmap<std::tuple<std::string, std::shared_ptr<ASTNode>, std::string>, std::shared_ptr<ASTNode>>(
        [](const std::tuple<std::string, std::shared_ptr<ASTNode>, std::string>& tuple) {
            auto node = std::make_shared<ASTNode>(ASTNode::Type::Parentheses, "");
            node->left = std::get<1>(tuple);
            return node;
        },
        sequence(parseLit("("), expressionParser(), parseLit(")")));
}

ps::Parser<std::shared_ptr<ASTNode>> expressionParser() {
    using namespace ps;
    return choice(
        parameterParser(),
        numberParser(),
        operatorParser(),
        parenthesesParser());
}

// Evaluate the AST using the parameter map
int evaluateAST(const std::shared_ptr<ASTNode>& node, const std::map<std::string, int>& params) {
    if (!node) {
        throw std::runtime_error("Invalid AST node");
    }

    switch (node->type) {
        case ASTNode::Type::Parameter: {
            auto it = params.find(node->value);
            if (it == params.end()) {
                throw std::runtime_error("Undefined parameter: " + node->value);
            }
            return it->second;
        }
        case ASTNode::Type::Number:
            return node->number;
        case ASTNode::Type::Operator: {
            int leftValue = evaluateAST(node->left, params);
            int rightValue = evaluateAST(node->right, params);

            if (node->value == ">=") {
                return leftValue >= rightValue;
            } else if (node->value == "+") {
                return leftValue + rightValue;
            } else if (node->value == "*") {
                return leftValue * rightValue;
            } else if (node->value == "/") {
                if (rightValue == 0) {
                    throw std::runtime_error("Division by zero");
                }
                return leftValue / rightValue;
            } else {
                throw std::runtime_error("Unknown operator: " + node->value);
            }
        }
        case ASTNode::Type::Parentheses:
            return evaluateAST(node->left, params);
        default:
            throw std::runtime_error("Unknown AST node type");
    }
}

void SamplesTest::personInfoParserTest()
{
    using namespace ps;

    auto src = "John,Doe,30,123-45  -    6789}";
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

void SamplesTest::customLanguageParserTest()
{
    using namespace ps;

    // Expressions example: `[p1] >= ([p2]+1) * [p3]/[p4]`
    // Where p1, p2, p3, p4 are parameter names (capitalized words), and the operators are >=, +, *, /, and parentheses.
    // Actual parameters are defined separately as a map.
    // When evaluating, parameter names will be replaced by their values from the map, and the expression will be evaluated according to operator precedence.

    // Example parameter map
    std::map<std::string, int> params = {
        {"P1", 5},
        {"P2", 10},
        {"P3", 3},
        {"P4", 2}
    };

    // Parse the expression
    std::string expression = "[P1] >= ([P2]+1) * [P3]/[P4]";
    ParserRuntime runtime(expression, State{});
    Parser<std::shared_ptr<ASTNode>> parser = expressionParser();

    ParserResult<std::shared_ptr<ASTNode>> result = parseWithRuntime(runtime, parser);

    QVERIFY(isRight(result));

    // Evaluate the AST
    std::shared_ptr<ASTNode> ast = getParseSucceeded(result).parsed;
    int evaluationResult = evaluateAST(ast, params);

    // Verify the result
    QVERIFY(evaluationResult == 1); // 5 >= (10+1) * 3/2 == 5 >= 16.5 == false
}

