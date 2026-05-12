#pragma once

#include "concepts.h"
#include "compose.h"
#include <ostream>

namespace nn {

CONCEPT(Tristate);
CONCEPT(NotMatchedYet);
CONCEPT(MatchedRegular);
CONCEPT(MatchedFinal);

enum class tristate_kind {
    not_matched_yet,
    matched_regular,
    matched_final,
    matched_final_halted,
};

template<tristate_kind kind, class T> struct tristate {
    static constexpr bool NMY = kind == tristate_kind::not_matched_yet;
    static constexpr bool MAT = !NMY;
    static constexpr bool REG = kind == tristate_kind::matched_regular;
    static constexpr bool FIN = !NMY && !REG;

    REPRESENTS(Tristate);
    REPRESENTS_COND(NotMatchedYet, NMY);
    REPRESENTS_COND(MatchedRegular, REG);
    REPRESENTS_COND(MatchedFinal, FIN);

    using type = T;
    T value;
    static constexpr bool is_matched = MAT;

    // comparable with itself or with same kind

    constexpr bool operator == (const tristate&) const
        requires std::equality_comparable<T>
        = default;

    template<class T1>
    constexpr bool operator == (const tristate<kind, T1>& other) const
        requires std::equality_comparable<T>
        { return value == other.value; }

    template<tristate_kind k1, class T1>
    constexpr bool operator == (const tristate<k1, T1>& other) const
        { return false; }

    // not_matched_yet will be passed to next rule in the sequence
    constexpr decltype(auto) operator >> (auto&& f) &&     requires NMY { return f(std::move(*this)); }
    constexpr decltype(auto) operator >> (auto&& f) const& requires NMY { return f(*this); }
    // others break the sequence
    constexpr decltype(auto) operator >> (auto&& f) &&     requires MAT { return std::move(*this); }
    constexpr decltype(auto) operator >> (auto&& f) const& requires MAT { return (*this); }

    // not-matched-yet result of (nmy >> p1 >> p2 >> p3) is nmy itself
    constexpr decltype(auto) commit_alts() &&     requires NMY { return std::move(*this); }
    constexpr decltype(auto) commit_alts() const& requires NMY { return *this; }
    // others should copy
    constexpr auto           commit_alts() &&     requires MAT { return std::move(*this); }
    constexpr auto           commit_alts() const& requires MAT { return *this; }

    // not_matched_yet breaks the loop
    constexpr auto commit_loop() &&     requires NMY { return tristate<tristate_kind::matched_final_halted, T>{std::move(value)}; }
    constexpr auto commit_loop() const& requires NMY { return tristate<tristate_kind::matched_final_halted, T>{value}; }
    // matched_regular restarts the loop
    constexpr auto commit_loop() &&     requires REG { return tristate<tristate_kind::not_matched_yet, T>{std::move(value)}; }
    constexpr auto commit_loop() const& requires REG { return tristate<tristate_kind::not_matched_yet, T>{value}; }
    // others break the loop with themselves
    constexpr auto commit_loop() &&     requires FIN { return tristate{std::move(value)}; }
    constexpr auto commit_loop() const& requires FIN { return tristate{value}; }

    // rebind the wrapper with another value
    template<class V>
    constexpr auto rebind(V v) const { return tristate<kind, V>{std::move(v)}; }
};

template<class T> using not_matched_yet = tristate<tristate_kind::not_matched_yet, T>;
template<class T> using matched_regular = tristate<tristate_kind::matched_regular, T>;
template<class T> using matched_final = tristate<tristate_kind::matched_final, T>;
template<class T> using matched_final_halted = tristate<tristate_kind::matched_final_halted, T>; // special case of final

static_assert(Tristate<not_matched_yet<int>>);
static_assert(NotMatchedYet<not_matched_yet<int>>);


} // namespace nn
