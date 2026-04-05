#pragma once

#include "value_type.h"
#include <cstddef>
#include <concepts>

namespace ss {

    // size_t as a compile time constant
    template<size_t N> using size_valuetype = valuetype<N>;
    template<size_t N> constexpr auto size_value = value<N>;

    template<class T> concept SizeValueType = ValueType<T> && std::same_as<size_t, typename T::type>;

    namespace literals {

        inline constexpr auto operator""_n(unsigned long long n) { return size_t(n); }

    }  // namespace literals

}  // namespace ss
