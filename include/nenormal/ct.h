#pragma once

// compile time constants

#include "concepts.h"

template<auto V> struct ct {
    using type = decltype(V);
    static constexpr auto value = V;
    REPRESENTS(Ct)
    friend std::ostream& operator << (std::ostream& os, ct const& v) {
        os << "ct<" << v.value << ">";
        return os;
    }
};
CONCEPT(Ct)
