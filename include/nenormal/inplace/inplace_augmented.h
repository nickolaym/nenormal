#pragma once

#include "../concepts.h"
#include <string>

namespace nn {

CONCEPT(InplaceAugmentation)

struct inplace_empty {
    REPRESENTS(InplaceAugmentation);
    constexpr void operator()(auto p, std::string const& t) const {}

    constexpr bool operator == (inplace_empty const&) const = default;
};

template<class A>
struct inplace_passed {
    REPRESENTS(InplaceAugmentation);
    A a;

    constexpr void operator()(auto p, std::string const& t) const {}

    constexpr bool operator == (inplace_passed const&) const = default;

    constexpr bool operator == (InplaceAugmentation auto const& other) const
        requires requires { a == other.a; }
    { return a == other.a; }

    friend constexpr bool operator == (InplaceAugmentation auto const& lhs, inplace_passed const& rhs)
        requires requires { lhs.a == rhs.a; }
    { return lhs.a == rhs.a; }
};

template<class F>
struct inplace_side_effect {
    REPRESENTS(InplaceAugmentation);
    F f;
    constexpr void operator()(auto p, std::string const& t) const { f(p, t); }
    constexpr bool operator == (inplace_side_effect const&) const { return true; } // do not compare functions
};

template<class A, class F>
struct inplace_cumulative_effect {
    REPRESENTS(InplaceAugmentation);
    // order of members is unfied with cumulative_effect<A,F>
    A a;
    F f; // A f(A&& a, auto p, std::string const& t)
    constexpr void operator()(auto p, std::string const& t) { a = f(a, p, t); }

    constexpr bool operator == (inplace_cumulative_effect const& other) const
        requires requires { a == other.a; }
    { return a == other.a; } // do not compare functions
};

template<class F, class A>
struct inplace_modification_effect {
    REPRESENTS(InplaceAugmentation);
    A a;
    F f; // void f(A& a, auto p, std::string const& t)
    constexpr void operator()(auto p, std::string const& t) { f(a, p, t); }

    constexpr bool operator == (inplace_modification_effect const& other) const
        requires requires { a == other.a; }
    { return a == other.a; } // do not compare functions
};

CONCEPT(InplaceAugmented);

template<InplaceAugmentation A>
struct inplace_augmented_text {
    REPRESENTS(InplaceAugmented);
    std::string text;
    A aux;

    constexpr bool operator == (inplace_augmented_text const& other) const
        requires std::equality_comparable<A>
    { return text == other.text && aux == other.aux; }

    constexpr bool operator == (InplaceAugmented auto const& other) const
        requires requires { aux == other.aux; }
    { return text == other.text && aux == other.aux; }
};

constexpr std::string& inplace_extract_text(std::string& t) { return t; }
constexpr std::string& inplace_extract_text(InplaceAugmented auto& t) { return t.text; }

constexpr void inplace_update_text(std::string& t, auto p) {}
constexpr void inplace_update_text(InplaceAugmented auto& t, auto p) { t.aux(p, t.text); }

} // namespace nn
