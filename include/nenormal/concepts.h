#pragma once

// all for concepts

#include <concepts>
#include <type_traits>
#include <cstddef>

template<class T> concept ValueType = !std::is_const_v<T> && !std::is_reference_v<T>;

#define REPRESENTS(Name) static constexpr bool this_is_##Name = true;
#define CONCEPT(Name) template<class T> concept Name = ValueType<T> && (T::this_is_##Name == true);
