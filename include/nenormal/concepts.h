#pragma once

// all for concepts

#include <concepts>
#include <type_traits>
#include <cstddef>

namespace nn {

template<class T> concept ValueType = !::std::is_const_v<T> && !::std::is_reference_v<T>;

} // namespace nn

#define CONCEPT_VALUE(Name) is_##Name##_v

// defines a concept of a structure types
#define CONCEPT(Name) \
    template<class T> \
    concept Name = \
        (::nn::ValueType<T>) && \
        (T::CONCEPT_VALUE(Name) == true) \
        ;

// assertion that the concept is defined
// (compiler errors, if not true)
#define CONCEPT_IS_DEFINED(Name) (!Name<void>)

#define REPRESENTS_COND(Name, cond) \
    static_assert(CONCEPT_IS_DEFINED(Name)); \
    static constexpr bool CONCEPT_VALUE(Name) = (cond);

#define REPRESENTS(Name) REPRESENTS_COND(Name, true)
