C++ Monadic Parsers library
===========================

Monadic parsers similar to Haskell's Parsec.


**Additional materials**
- [Monadic Parsers in C++ (Talk, Rus)](https://www.youtube.com/watch?v=q39PHTJDaLE) | [Slides (Eng)](https://docs.google.com/presentation/d/1zlwKBX8-DYVWUYmzvmKm7ggDVBugEJzY6OFSWjeQOA4/edit?usp=sharing) | My talk about this implementation at C++ Russia 2019 Moscow.
- [cpp_stm_free](https://github.com/graninas/cpp_stm_free) | Software Transactional Memory with the same Free Monads approach

Features
--------
- Monadic parser combinators in C++ (like Haskell's Parsec)
- Build complex parsers by combining simple ones
- Parsers for digits, letters, literals, sequences, separated lists, etc.
- Combinators: `many`, `many1`, `sepBy1`, `between`, `count`, `discard`, `both`, `left`, `right`, etc.
- Easy mapping of parsed tuples into user-defined structs
- Safe runtime with error handling and position tracking

Examples
--------

Parse a single digit:
```cpp
using namespace ps;
ParserRuntime runtime("1", State{});
ParserResult<Char> result = parseWithRuntime<Char>(runtime, digit);
if (isRight(result)) {
    Char c = getParseSucceeded(result).parsed; // c == '1'
}
```

Parse a capitalized word:
```cpp
ParserL<char> firstChar = upper;
ParserL<std::list<char>> restChars = many(lower);
auto seqp = sequence(firstChar, restChars);
auto parser = merge(seqp);
```

Parse a person info line:
```cpp
struct PersonInfo { std::string firstName, lastName; int age; std::string ssn; };
auto seqp = sequence(
    firstNameParser(), skip(comma),
    lastNameParser(), skip(comma),
    ageParser(), skip(comma),
    ssnParser()
);
auto parser = as<PersonInfo>(seqp);
```

Architecture Overview
---------------------
- **ParserL**: Main parser type, parameterized by result type.
- **ParserResult**: Holds either a successful parse or failure with error message and position.
- **Combinators**: Functions like `many`, `sequence`, `bind`, `fmap`, etc., to build complex parsers.
- **ParserRuntime**: Holds input string, state, and error messages. Used for running parsers safely.
- **State**: Simple user state (not implemented yet).
- **Error Handling**: Failures are tracked with messages and positions.
- **Tuple Mapping**: Results from sequences can be mapped into user structs with `as<T>`.

This library lets you write expressive and type-safe parsers in C++ using a functional style, similar to Haskell's Parsec. You can parse structured text, validate formats, and build custom DSLs with clear and composable code.
