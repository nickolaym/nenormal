#pragma once

#include "concepts.h"
#include "compose.h"
#include <ostream>

namespace nn {

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

    static constexpr bool is_matched = false;

    // not_matched_yet will be passed to next rule in the sequence
    constexpr decltype(auto) operator >> (auto&& f) && { return f(std::move(*this)); }
    constexpr decltype(auto) operator >> (auto&& f) const& { return f(*this); }

    // not-matched-yet result of (nmy >> p1 >> p2 >> p3) is nmy itself
    constexpr decltype(auto) commit_alts() && { return std::move(*this); }
    constexpr decltype(auto) commit_alts() const& { return *this; }

    // not_matched_yet breaks the loop
    constexpr auto commit_loop() && { return matched_final_halted{std::move(value)}; }
    constexpr auto commit_loop() const& { return matched_final_halted{value}; }

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

    static constexpr bool is_matched = true;

    // matched_regular breaks the sequence
    // return by reference to avoid copying
    constexpr decltype(auto) operator >> (auto&& f) && { return std::move(*this); }
    constexpr decltype(auto) operator >> (auto&& f) const& { return *this; }

    // matched-somewhere result of (nmy >> p1 >> p2 >> p3) is temporary
    constexpr auto commit_alts() && { return std::move(*this); }
    constexpr auto commit_alts() const& { return *this; }

    // matched_regular restarts the loop
    constexpr auto commit_loop() && { return not_matched_yet{std::move(value)}; }
    constexpr auto commit_loop() const& { return not_matched_yet{value}; }

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

    static constexpr bool is_matched = true;

    // matched_final breaks the sequence
    // return by reference to avoid copying
    constexpr decltype(auto) operator >> (auto&& f) && { return std::move(*this); }
    constexpr decltype(auto) operator >> (auto&& f) const& { return *this; }

    // matched-somewhere result of (nmy >> p1 >> p2 >> p3) is temporary
    constexpr auto commit_alts() && { return std::move(*this); }
    constexpr auto commit_alts() const& { return *this; }

    // matched_final breaks the loop
    constexpr auto commit_loop() && { return std::move(*this); }
    constexpr auto commit_loop() const& { return *this; }

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

    static constexpr bool is_matched = true;

    // matched_final_halted breaks the sequence
    constexpr decltype(auto) operator >> (auto&& f) && { return std::move(*this); }
    constexpr decltype(auto) operator >> (auto&& f) const& { return *this; }

    // matched-somewhere result of (nmy >> p1 >> p2 >> p3) is temporary
    constexpr auto commit_alts() && { return std::move(*this); }
    constexpr auto commit_alts() const& { return *this; }

    // matched_final breaks the loop
    constexpr auto commit_loop() && { return std::move(*this); }
    constexpr auto commit_loop() const& { return *this; }

    // return Tristate of same kind, with new value
    constexpr auto rebind(auto v) const { return matched_final_halted<decltype(v)>{v}; }
};

} // namespace nn
