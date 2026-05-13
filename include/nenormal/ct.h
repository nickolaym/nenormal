#pragma once

// compile time constants

#include "concepts.h"
#include <iostream>

namespace nn {

CONCEPT_WITH_TYPE(Ct)

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

template<auto V> constexpr auto ctv = ct<V>{};

} // namespace nn
