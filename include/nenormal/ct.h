#pragma once

// compile time constants

#include "concepts.h"
#include <iostream>

template<auto V> struct ct {
    using type = std::remove_const_t<decltype(V)>;
    static constexpr auto value = V;
    REPRESENTS(Ct)
    friend std::ostream& operator << (std::ostream& os, ct const& v) {
        os << "ct<" << v.value << ">";
        return os;
    }
    constexpr bool operator == (ct const&) const = default;
    template<auto U> constexpr bool operator == (ct<U> const&) const { return V == U; }
};
CONCEPT(Ct)

template<class T, class U> concept CtOf = Ct<T> && std::same_as<typename T::type, U>;

template<auto V> constexpr auto ctv = ct<V>{};