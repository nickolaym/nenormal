#pragma once

#include "concepts.h"
#include "compose.h"
#include <ostream>

CONCEPT(NotMatchedYet);
CONCEPT(MatchedRegular);
CONCEPT(MatchedFinal);
template<class T> concept Tristate = MatchedRegular<T> || MatchedFinal<T> || NotMatchedYet<T>;

template<class T> struct not_matched_yet;
template<class T> struct matched_regular;
template<class T> struct matched_final;

template<class T> struct not_matched_yet {
    REPRESENTS(NotMatchedYet);
    using type = T;
    T value;
    constexpr bool operator == (not_matched_yet const&) const = default;
    friend std::ostream& operator << (std::ostream& os, not_matched_yet const& v) {
        return os << "not_matched_yet{" << v.value << "}";
    }

    // not_matched_yet will be passed to next rule in the sequence
    constexpr decltype(auto) operator >> (auto&& f) const { return f(value); }

    // not_matched_yet breaks the loop
    constexpr decltype(auto) commit() const { return matched_final{value}; }
};

template<class T> struct matched_regular {
    REPRESENTS(MatchedRegular);
    using type = T;
    T value;
    constexpr bool operator == (matched_regular const&) const = default;
    friend std::ostream& operator << (std::ostream& os, matched_regular const& v) {
        return os << "matched_regular{" << v.value << "}";
    }

    // matched_regular breaks the sequence
    constexpr decltype(auto) operator >> (auto&& f) const { return *this; }

    // matched_regular restarts the loop
    constexpr decltype(auto) commit() const { return not_matched_yet{value}; }
};

template<class T> struct matched_final {
    REPRESENTS(MatchedFinal);
    using type = T;
    T value;
    constexpr bool operator == (matched_final const&) const = default;
    friend std::ostream& operator << (std::ostream& os, matched_final const& v) {
        return os << "matched_final{" << v.value << "}";
    }

    // matched_final breaks the sequence
    constexpr decltype(auto) operator >> (auto&& f) const { return *this; }

    // matched_final breaks the loop
    constexpr decltype(auto) commit() const { return *this; }
};
