#pragma once

#include "concepts.h"
#include "ct.h"

#include <string_view>
#include <iostream>
#include <iomanip>

namespace nn {

template<size_t N> using charbuf = char[N];

template<size_t N> struct str {
    static_assert(N > 0); // nul-terminated string
    charbuf<N> value = {};
    constexpr str() = default;
    constexpr str(const charbuf<N>& v) { std::copy(std::begin(v), std::end(v), std::begin(value)); }

    REPRESENTS(Str)
    friend std::ostream& operator << (std::ostream& os, str const& v) {
        os << std::quoted(v.view()) << "_ss";
        return os;
    }

    constexpr auto begin() { return std::begin(value); }
    constexpr auto begin() const { return std::begin(value); }
    constexpr auto end() { return std::begin(value) + size(); }
    constexpr auto end() const { return std::begin(value) + size(); }

    constexpr size_t size() const { return N-1; }
    constexpr bool empty() const { return size() == 0; }

    constexpr operator std::string_view() const { return view(); }
    constexpr std::string_view view() const { return {begin(), end()}; }
    constexpr auto& operator[](size_t i) { return value[i]; }
    constexpr auto& operator[](size_t i) const { return value[i]; }

    constexpr bool operator == (const str& rhs) const = default;
    template<size_t M> constexpr bool operator == (const str<M>& rhs) const { return false; }
};
// CTAD
template<size_t N> str(const charbuf<N>&) -> str<N>;

CONCEPT(Str)
template<class T> concept CtStr = Ct<T> && Str<typename T::type>;

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
