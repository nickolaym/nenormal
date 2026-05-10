#pragma once

#include "../concepts.h"
#include <iostream>
#include <cassert>

namespace nn {

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

} // namespace nn
