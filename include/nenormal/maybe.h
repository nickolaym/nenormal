#pragma once

#include "concepts.h"
#include <iostream>

namespace nn {

CONCEPT(Nothing); // stops a chain
CONCEPT_WITH_TYPE(Just); // continues a chain
template<class T> concept Maybe = Nothing<T> || Just<T>;

struct nothing;
template<class T> struct just;

struct nothing {
    REPRESENTS(Nothing);

    constexpr bool operator==(const nothing&) const = default;
    constexpr bool operator==(const Just auto&) const { return false; }

    friend std::ostream& operator << (std::ostream& ost, const nothing&) {
        return ost << "nothing";
    }

    constexpr operator bool() const { return false; }
    constexpr auto then(auto f, auto e) const { return e; }
    constexpr auto then_else(auto f, auto g) const { return g(); } // lazy version of then
};

template<class T>
struct just {
    REPRESENTS(Just);
    using type = T;
    T value;

    constexpr bool operator==(const just&) const = default;
    constexpr bool operator==(const nothing&) const { return false; }
    constexpr bool operator==(const Just auto&) const { return false; } // TODO: comparable types

    friend std::ostream& operator << (std::ostream& ost, const just& j) {
        return ost << "just{" << j.value << "}";
    }

    constexpr operator bool() const { return true; }
    constexpr auto then(auto f, auto e) const { return f(value); }
    constexpr auto then_else(auto f, auto g) const { return f(value); }
};

template<class T, class V> concept JustOf = Just<T> && std::same_as<T, V>;
template<class T, class V> concept MaybeOf = Nothing<T> || JustOf<T, V>;

} // namespace nn
