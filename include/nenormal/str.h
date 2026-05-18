#pragma once

#include "concepts.h"
#include "ct.h"

#include <string_view>
#include <iostream>
#include <iomanip>

namespace nn {

CONCEPT(Str)

template<class T> concept CtStr = CtOfTraits<T, is_Str>;
CONCEPT_TYPECHECKER(CtStr)

// note that N here includes the nul-terminator
template<size_t N> using charbuf = char[N];

// note that N here excludes the nul-terminator.
// so it's impossible to deduce N from the constructor implicitly,
// and it requires explicit CTAD
template<size_t N> struct str {
    charbuf<N + 1> value = {};
    constexpr str() = default;
    constexpr str(const charbuf<N + 1>& v) {
        std::copy(std::begin(v), std::end(v) - 1, std::begin(value));
        value[N] = 0;
    }

    REPRESENTS(Str)
    friend std::ostream& operator << (std::ostream& os, str const& v) {
        os << std::quoted(v.view()) << "_ss";
        return os;
    }

    constexpr auto begin() { return std::begin(value); }
    constexpr auto begin() const { return std::begin(value); }
    constexpr auto end() { return std::begin(value) + size(); }
    constexpr auto end() const { return std::begin(value) + size(); }

    constexpr static size_t size() { return N; }
    constexpr static bool empty() { return size() == 0; }

    constexpr operator std::string_view() const { return view(); }
    constexpr std::string_view view() const { return {begin(), end()}; }
    constexpr auto& operator[](size_t i) { return value[i]; }
    constexpr auto& operator[](size_t i) const { return value[i]; }

    constexpr bool operator == (const str& rhs) const = default;
    template<size_t M> constexpr bool operator == (const str<M>& rhs) const { return false; }
};
// CTAD
template<size_t N> str(const charbuf<N>&) -> str<N - 1>;

namespace literals {

// string literals
template<str s> constexpr auto operator""_ss() { return s; }
template<str s> constexpr auto operator""_cts() { return ct<s>{}; }

} // namespace literals

} // namespace nn

// functional form of string literals
#define STR(s) (::nn::str{s}) // s##_ss
#define CTSTR(s) (::nn::ct<STR(s)>{}) // s##_cts
// note that these macros return rvalues, not constexpr lvalues
