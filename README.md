C++ Monadic Parsers library
===========================

Experimental implementation of monadic parsers similar to Haskell's Parsec but in C++ using Free monads as engine.

The design of the library is not finished for now. Combinators are currently divided into two layers: `ParserL` and `ParserT`. These layers should be unified to eliminate the difference in usage.

**Additional materials**
- [Monadic Parsers in C++ (Talk, Rus)](https://www.youtube.com/watch?v=q39PHTJDaLE) | [Slides (Rus)](https://docs.google.com/presentation/d/1zlwKBX8-DYVWUYmzvmKm7ggDVBugEJzY6OFSWjeQOA4/edit?usp=sharing) | My talk about this implementation at C++ Russia 2019 Moscow.
- [cpp_stm_free](https://github.com/graninas/cpp_stm_free) | Software Transactional Memory with the same Free Monads approach

Requirements
------------

- GCC 7.2 (C++17)

Samples
-------

### Monadic sequencing

```cpp
struct R
{
    Char dg0;
    Char ch1;
    Char ch2;
};

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
```

### `alt` combinator

```cpp
ParserResult<Char> result1 = parse(alt(upperPL, lowerPL), "A");
ParserResult<Char> result2 = parse(alt(upperPL, lowerPL), "a");

QVERIFY(isRight(result1));
QVERIFY(isRight(result2));
QVERIFY(getParsed(result1) == 'A');
QVERIFY(getParsed(result2) == 'a');
```

### `try` and `safe` combinators

```cpp
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
```

### `many` combinator

```cpp
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
```
