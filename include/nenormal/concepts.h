#pragma once

// all for concepts

#include <concepts>
#include <type_traits>
#include <cstddef>

#define REPRESENTS(Name) static constexpr bool this_is_##Name = true;
#define CONCEPT(Name) template<class T> concept Name = (T::this_is_##Name == true);
