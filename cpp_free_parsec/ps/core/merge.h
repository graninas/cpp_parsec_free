#ifndef PS_CORE_MERGE_H
#define PS_CORE_MERGE_H

#include <tuple>
#include <string>
#include <list>
#include <vector>
#include <type_traits>

#include "types.h"
#include "parser/functor.h" // for fmap

namespace ps {
namespace core {

// Helper: tuple to string for (char, list<char>) or (char, vector<char>)
template <typename Tuple>
struct TupleToString;

// (char, list<char>)
template <>
struct TupleToString<std::tuple<char, std::list<char>>> {
    static std::string convert(const std::tuple<char, std::list<char>>& t) {
        std::string s;
        s += std::get<0>(t);
        for (char c : std::get<1>(t)) s += c;
        return s;
    }
};

// (char, vector<char>)
template <>
struct TupleToString<std::tuple<char, std::vector<char>>> {
    static std::string convert(const std::tuple<char, std::vector<char>>& t) {
        std::string s;
        s += std::get<0>(t);
        for (char c : std::get<1>(t)) s += c;
        return s;
    }
};

// (char, char)
template <>
struct TupleToString<std::tuple<char, char>> {
    static std::string convert(const std::tuple<char, char>& t) {
        std::string s;
        s += std::get<0>(t);
        s += std::get<1>(t);
        return s;
    }
};

// Generic TupleToString for any tuple of chars/strings
namespace detail {
    inline void tupleToStringElem(std::string& s, char c) { s += c; }
    inline void tupleToStringElem(std::string& s, const std::string& str) { s += str; }
    template <typename Tuple, std::size_t... Is>
    std::string tupleToStringImpl(const Tuple& t, std::index_sequence<Is...>) {
        std::string s;
        (void)std::initializer_list<int>{
            (tupleToStringElem(s, std::get<Is>(t)), 0)...
        };
        return s;
    }
}

template <typename... Ts>
struct TupleToString<std::tuple<Ts...>> {
    static std::string convert(const std::tuple<Ts...>& t) {
        return detail::tupleToStringImpl(t, std::index_sequence_for<Ts...>{});
    }
};

// Add more specializations as needed

// Helper trait to extract result type from Parser<T>
template <typename Parser>
struct parser_result_type;

template <template <typename> class Parser, typename T>
struct parser_result_type<Parser<T>> {
    using type = T;
};

// Generic merge for std::string
// Accepts a parser returning a tuple, returns a parser returning std::string
template <typename ParserTuple>
auto merge(ParserTuple p) {
    using TupleType = typename parser_result_type<ParserTuple>::type;
    return fmap<TupleType, std::string>(
        [](const TupleType& t) { return TupleToString<TupleType>::convert(t); }, p);
}

// Helper: is_tuple type trait
template <typename T>
struct is_tuple : std::false_type {};
template <typename... Ts>
struct is_tuple<std::tuple<Ts...>> : std::true_type {};
// Generic mergeTo<T> for containers of char or tuples
// Special handling for std::string
// Accepts a parser returning a container or tuple, returns a parser returning T
template <typename T, typename ParserContainer>
auto mergeTo(ParserContainer p) {
    using ValueType = typename parser_result_type<ParserContainer>::type;
    return fmap<ValueType, T>(
        [](const ValueType& chars) {
            if constexpr (std::is_same<T, std::string>::value) {
                if constexpr (is_tuple<ValueType>::value) {
                    return TupleToString<ValueType>::convert(chars);
                } else {
                    return std::string(chars.begin(), chars.end());
                }
            } else if constexpr (std::is_same<T, int>::value) {
                return std::stoi(std::string(chars.begin(), chars.end()));
            } else if constexpr (std::is_same<T, double>::value) {
                return std::stod(std::string(chars.begin(), chars.end()));
            } else {
                static_assert(sizeof(T) == 0, "mergeTo: unsupported type");
            }
        },
        p
    );
}

} // namespace core
} // namespace ps

#endif // PS_CORE_MERGE_H
