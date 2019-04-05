#ifndef PS_CHURCH_PS_H
#define PS_CHURCH_PS_H

#include "../types.h"
#include "../context.h"
#include "../impl/runtime.h"
#include "parserl.h"
#include "interpreter.h"

namespace ps
{
namespace church
{

template <typename A, typename B>
ParserL<B> bind(const ParserL<A>& ma,
                const std::function<ParserL<B>(A)>& f)
{
    std::cout << "bind\n";
    ParserL<B> n;
    n.runF = [=](const std::function<Any(B)>& p,
                 const std::function<Any(psf::ParserF<Any>)>& r)
    {
        std::function<Any(A)> fst = [=](const A& a)
        {
            std::cout << "bind -> \\runF -> \\fst\n";
            ParserL<B> internal = f(a);
            Any res = internal.runF(p, r);  // Any == B
            return res;
        };

        std::cout << "bind -> \\runF -> ma.runF()\n";
        Any runFResult = ma.runF(fst, r);
        std::cout << "bind -> \\runF -> success\n";
        return runFResult;
    };

    std::cout << "bind -> success\n";
    return n;
}

template <typename A>
ParserL<A> join(const ParserL<ParserL<A>>& mma)
{
    return bind<ParserL<A>, A>(mma, [](const ParserL<A>& ma) { return ma; });
}

template <typename A>
ParserL<A> pure(const A& a, const std::string& name = "")
{
    std::cout << "pure " + name + " \n";
    ParserL<A> n;
    n.runF = [=](const std::function<Any(A)>& p,
                 const std::function<Any(psf::ParserF<Any>)>&)
    {
        std::cout << "pure " + name + " -> \\runF -> \\p()\n";
        Any pResult = p(a);
        std::cout << "pure " + name + " -> \\runF -> success\n";
        return pResult;
    };

    std::cout << "pure " + name + " -> success\n";
    return n;
}

template <typename A, template <typename> class Method>
ParserL<A> wrap(const Method<A>& method, const std::string& name = "")
{
    std::cout << "wrap " + name + " \n";
    ParserL<A> n;

    n.runF = [=](const std::function<Any(A)>& p,
                 const std::function<Any(psf::ParserF<Any>)>& r)
    {
        std::cout << "wrap " + name + " -> \\runF -> fmap\n";
        psf::ParserF<A> f { method };
        psf::ParserF<Any> mapped = psf::fmap<A, Any>(p, f);
        std::cout << "wrap " + name + " -> \\runF -> \\r()\n";
        Any rResult = r(mapped);
        std::cout << "wrap " + name + " -> \\runF -> success\n";
        return rResult;
    };

    std::cout << "wrap " + name + " -> success\n";
    return n;
}

ParserL<Digit> parseDigit()
{
    return wrap(psf::ParseDigit<Digit>{ id }, "[ParseDigit]");
}

ParserL<Char> parseLowerCaseChar()
{
    return wrap(psf::ParseLowerCaseChar<Char>{ id }, "[ParseLowerCaseChar]");
}

ParserL<Char> parseUpperCaseChar()
{
    return wrap(psf::ParseUpperCaseChar<Char>{ id }, "[ParseUpperCaseChar]");
}

const ParserL<Digit> digit = parseDigit();
const ParserL<Char> lowerCaseChar = parseLowerCaseChar();
const ParserL<Char> upperCaseChar = parseUpperCaseChar();

/// ParserL evaluation

template <typename A>
ps::ParseResult<A> parse(
        const ParserL<A>& psl,
        const std::string& s)
{
    std::cout << "parse\n";

    if (s.empty())
        return { ParseError { "Source string is empty." }};

    std::cout << "parse -> runParserL\n";
    ParserRuntime runtime(s, 0);
    RunResult<A> runResult = runParserL<A>(runtime, psl);

    if (isLeft(runResult.result))
    {
        std::cout << "parse -> isLeft\n";
        return std::get<ParseError>(runResult.result);
    }

    std::cout << "parse -> success\n";
    return { std::get<A>(runResult.result) };
}

template <typename T>
ParserL<Many<T>> many(const ParserL<T>& parser)
{

}


} // namespace church
} // namespace ps

#endif // PS_CHURCH_PS_H
