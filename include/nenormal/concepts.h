#pragma once

// all for concepts

#include <concepts>
#include <type_traits>
#include <cstddef>

namespace nn {

template<class T> concept ValueType = !::std::is_const_v<T> && !::std::is_reference_v<T>;

} // namespace nn

// Foo_concept_probe is a helper structure
// to be passed to an overloaded static member represent_concept(Foo_concept_probe)
// It allows constructing a qualified name, like ::some_namespace::Foo_concept_probe
#define CONCEPT_PROBE_NAME_(Name) Name##_concept_probe
#define CONCEPT_PROBE_NAME(Name) CONCEPT_PROBE_NAME_(Name)

// defines a concept of a structure types
#define CONCEPT(Name) \
    struct CONCEPT_PROBE_NAME(Name) : std::true_type{}; \
    template<class T> \
    concept Name = \
        (::nn::ValueType<T>) && \
        requires { T::represents_concept(CONCEPT_PROBE_NAME(Name){}); } \
        ;

// assertion that the concept is defined
// (compiler errors, if not true)
#define CONCEPT_IS_DEFINED(MaybeQualifiedName) (!MaybeQualifiedName<void>)

#define REPRESENTS_COND(MaybeQualifiedName, cond) \
    static_assert(CONCEPT_IS_DEFINED(MaybeQualifiedName)); \
    static constexpr void represents_concept(CONCEPT_PROBE_NAME(MaybeQualifiedName)) \
        requires (cond);

#define REPRESENTS(MaybeQualifiedName) \
    static_assert(CONCEPT_IS_DEFINED(MaybeQualifiedName)); \
    static constexpr void represents_concept(CONCEPT_PROBE_NAME(MaybeQualifiedName));
