#pragma once

#include "concepts.h"
#include "compose.h"
#include <ostream>
#include <cassert>

CONCEPT(NotMatchedYet);
CONCEPT(MatchedRegular);
CONCEPT(MatchedFinal);
template<class T> concept Tristate =
    NotMatchedYet<T> || // will continue
    MatchedRegular<T> || // will stop matching, but restart the loop
    MatchedFinal<T>; // will stop the loop

template<class T> struct not_matched_yet;
template<class T> struct matched_regular;
template<class T> struct matched_final;
template<class T> struct matched_final_halted; // special case of final

template<class T> struct not_matched_yet {
    REPRESENTS(NotMatchedYet);
    using type = T;
    T value;
    constexpr bool operator == (not_matched_yet const&) const = default;
    constexpr bool operator == (Tristate auto const&) const { return false; }
    friend std::ostream& operator << (std::ostream& os, not_matched_yet const& v) {
        return os << "not_matched_yet{" << v.value << "}";
    }

    // not_matched_yet will be passed to next rule in the sequence
    constexpr decltype(auto) operator >> (auto&& f) const { return f(value); }

    // not_matched_yet breaks the loop
    constexpr decltype(auto) commit() const { return matched_final_halted{value}; }

    // return Tristate of same kind, with new value
    constexpr auto rebind(auto v) const { return not_matched_yet<decltype(v)>{v}; }
};

template<class T> struct matched_regular {
    REPRESENTS(MatchedRegular);
    using type = T;
    T value;
    constexpr bool operator == (matched_regular const&) const = default;
    constexpr bool operator == (Tristate auto const&) const { return false; }
    friend std::ostream& operator << (std::ostream& os, matched_regular const& v) {
        return os << "matched_regular{" << v.value << "}";
    }

    // matched_regular breaks the sequence
    constexpr decltype(auto) operator >> (auto&& f) const { return *this; }

    // matched_regular restarts the loop
    constexpr decltype(auto) commit() const { return not_matched_yet{value}; }

    // return Tristate of same kind, with new value
    constexpr auto rebind(auto v) const { return matched_regular<decltype(v)>{v}; }
};

template<class T> struct matched_final {
    REPRESENTS(MatchedFinal);
    using type = T;
    T value;
    constexpr bool operator == (matched_final const&) const = default;
    constexpr bool operator == (Tristate auto const&) const { return false; }
    friend std::ostream& operator << (std::ostream& os, matched_final const& v) {
        return os << "matched_final{" << v.value << "}";
    }

    // matched_final breaks the sequence
    constexpr decltype(auto) operator >> (auto&& f) const { return *this; }

    // matched_final breaks the loop
    constexpr decltype(auto) commit() const { return *this; }

    // return Tristate of same kind, with new value
    constexpr auto rebind(auto v) const { return matched_final<decltype(v)>{v}; }
};

template<class T> struct matched_final_halted {
    REPRESENTS(MatchedFinal);
    using type = T;
    T value;
    constexpr bool operator == (matched_final_halted const&) const = default;
    constexpr bool operator == (Tristate auto const&) const { return false; }
    friend std::ostream& operator << (std::ostream& os, matched_final_halted const& v) {
        return os << "matched_halted{" << v.value << "}";
    }

    // matched_final breaks the sequence
    constexpr decltype(auto) operator >> (auto&& f) const { return *this; }

    // matched_final breaks the loop
    constexpr decltype(auto) commit() const { return *this; }

    // return Tristate of same kind, with new value
    constexpr auto rebind(auto v) const { return matched_final_halted<decltype(v)>{v}; }
};

// for inplace performance

enum inplace_state {
    k_not_matched_yet,
    k_matched_regular,
    k_matched_final,
    k_matched_final_halted,
};

template<class T>
struct inplace_argument {
    REPRESENTS(Inplace);
    using type = T;
    T value; // rewriteable
    inplace_state state = k_not_matched_yet;

    bool operator == (const inplace_argument&) const = default;
    template<class U>
    bool operator == (const inplace_argument<U>& other) const {
        return value == other.value && state == other.state;
    }

    friend std::ostream& operator << (std::ostream& os, inplace_argument const& v) {
        static constexpr char const* names[] = {
            "inplace_not_matched_yet",
            "inplace_matched_regular",
            "inplace_matched_final",
            "inplace_matched_final_halted",
        };
        return os << names[v.state] << "{" << v.value << "}";
    }

    constexpr operator bool() const { return state != k_not_matched_yet; }

    constexpr bool updated_by(auto&& f) {
        if (!*this) {
            state = f(value);
        } else {
            assert(false);
        }
        return *this;
    }

    constexpr bool commit() {
        if (state == k_not_matched_yet) {
            state = k_matched_final_halted;
        } else if (state == k_matched_regular) {
            state = k_not_matched_yet;
        } else {
            // final -> final
            // halted -> halted
        }
        return *this;
    }
};
CONCEPT(Inplace);
template<class T, class V> concept InplaceOf = Inplace<T> && std::same_as<typename T::type, V>;
