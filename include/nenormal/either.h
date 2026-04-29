#pragma once

#include "concepts.h"
#include <iostream>

// monad Either

CONCEPT(Left); // stops a chain
CONCEPT(Right); // continues a chain
template<class T> concept Either = Left<T> || Right<T>;
// TODO: support runtime class with behavior of this monad.

template<class T> struct left_type {
    REPRESENTS(Left);
    using type = T;
    T value;
    constexpr Either auto operator >> (auto&& f) const { return *this; }

    friend std::ostream& operator << (std::ostream& ost, const left_type& t) {
        return ost << "left(" << t.value << ")";
    }

    constexpr bool operator == (const left_type& other) const = default;
    constexpr bool operator == (const Either auto& other) const { return false; }

    constexpr auto eitherConst(auto l, auto r) const {
        return r;
    }
    constexpr auto either(auto lf, auto rf) const {
        return lf(value);
    }
    constexpr auto eitherLifted(auto lf, auto rf) const {
        auto res = lf(value);
        return left_type<decltype(res)>{res};
    }
};
template<class T> struct right_type {
    REPRESENTS(Right);
    using type = T;
    T value;
    constexpr Either auto operator >> (auto&& f) const { return f(value); }

    friend std::ostream& operator << (std::ostream& ost, const right_type& t) {
        return ost << "right(" << t.value << ")";
    }

    constexpr bool operator == (const right_type& other) const = default;
    constexpr bool operator == (const Either auto& other) const { return false; }

    constexpr auto eitherConst(auto l, auto r) const {
        return r;
    }
    constexpr auto either(auto lf, auto rf) const {
        return rf(value);
    }
    constexpr auto eitherLifted(auto lf, auto rf) const {
        auto res = rf(value);
        return right_type<decltype(res)>{res};
    }
};

// named constructors of types
// because of ambiguity of C++ syntax
// left_type{x} means
// either left_type<decltype(x)>{.value = x}
// or copy of x, if x is left_type<T> itself.

// also we need them as object entities (see below)
constexpr auto left = []<class T>(T t) { return left_type<T>{t}; };
constexpr auto right = []<class T>(T t) { return right_type<T>{t}; };

// deconstructor of Either (left only, because right values are temporary)
constexpr auto fromLeft(Left auto lt) { return lt.value; }
