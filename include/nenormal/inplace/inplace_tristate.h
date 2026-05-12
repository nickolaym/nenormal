#pragma once

#include "../concepts.h"
#include <iostream>
#include <utility>

namespace nn {

CONCEPT(Inplace);
template<class T, class V> concept InplaceOf = Inplace<T> && std::same_as<typename T::type, V>;

template<class T>
struct inplace_argument {
    REPRESENTS(Inplace);
    using type = T;
    T value; // rewriteable
    tristate_kind kind = tristate_kind::not_matched_yet;

    bool operator == (const inplace_argument&) const = default;
    template<class U>
    bool operator == (const inplace_argument<U>& other) const {
        return value == other.value && kind == other.kind;
    }

    friend std::ostream& operator << (std::ostream& os, inplace_argument const& v) {
        static constexpr char const* names[] = {
            "inplace_not_matched_yet",
            "inplace_matched_regular",
            "inplace_matched_final",
            "inplace_matched_final_halted",
        };
        return os << names[(int)v.kind] << "{" << v.value << "}";
    }

    constexpr operator bool() const { return kind != tristate_kind::not_matched_yet; }

    constexpr bool updated_by(auto&& f) {
        if (kind != tristate_kind::not_matched_yet)
            throw std::runtime_error("unexpected update of already matched value");
        kind = f(value);
        return *this;
    }

    constexpr bool commit() {
        switch (kind) {
        case tristate_kind::not_matched_yet:
            kind = tristate_kind::matched_final_halted;
            break;
        case tristate_kind::matched_regular:
            kind = tristate_kind::not_matched_yet;
            break;
        case tristate_kind::matched_final:
        case tristate_kind::matched_final_halted:
            break;
        default:
            std::unreachable();
            break;
        }
        return *this;
    }
};

} // namespace nn
