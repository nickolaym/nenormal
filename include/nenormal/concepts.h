#pragma once

// all for concepts

#include <concepts>
#include <type_traits>
#include <cstddef>

namespace nn {

template<class T> concept ValueType = !::std::is_const_v<T> && !::std::is_reference_v<T>;

} // namespace nn

#define REPRESENTS_COND(Name, cond) static constexpr bool this_is_##Name = (cond);
#define REPRESENTS(Name) static constexpr bool this_is_##Name = true;
#define CONCEPT(Name) template<class T> concept Name = ::nn::ValueType<T> && (T::this_is_##Name == true);
