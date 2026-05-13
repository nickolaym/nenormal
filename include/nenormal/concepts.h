#pragma once

// all for concepts

#include <concepts>
#include <type_traits>
#include <cstddef>

namespace nn {

template<class T> concept ValueType = !::std::is_const_v<T> && !::std::is_reference_v<T>;

template<class T, class U>
concept HasTypeOfType = std::same_as<typename T::type, U>;

template<class T, template<class>class C>
concept HasTypeOfTraits = C<typename T::type>::value;

} // namespace nn

// Foo_concept_probe is a helper structure
// to be passed to an overloaded static member represent_concept(Foo_concept_probe)
// It allows constructing a qualified name, like ::some_namespace::Foo_concept_probe
#define CONCEPT_PROBE_NAME_(Name) Name##_concept_probe
#define CONCEPT_PROBE_NAME(Name) CONCEPT_PROBE_NAME_(Name)

// is_Foo is a metafuntion type - template<class>class : std::bool_constant
#define IS_CONCEPT_NAME_(Name) is_##Name
#define IS_CONCEPT_NAME(Name) IS_CONCEPT_NAME_(Name)
// is_Foo_v is corresponding metafunction value - tempate<class> constexpr bool
#define IS_CONCEPT_NAME_V_(Name) is_##Name##_v
#define IS_CONCEPT_NAME_V(Name) IS_CONCEPT_NAME_V_(Name)

// define arbitrary typechecker - struct is_Name and constexpr is_Name_v = (expr of T)
#define DEFINE_TYPECHECKER(Name, T, expr) \
    template<class T> \
    using IS_CONCEPT_NAME(Name) = std::bool_constant<(expr)>; \
    template<class T> \
    constexpr bool IS_CONCEPT_NAME_V(Name) = (expr);

// defines typechecker from given concept
#define CONCEPT_TYPECHECKER(Name) \
    DEFINE_TYPECHECKER(Name, T, (Name<T>))

// defines a concept of a structure types
#define CONCEPT(Name) \
    struct CONCEPT_PROBE_NAME(Name) {}; \
    template<class T> \
    concept Name = \
        /* (::nn::ValueType<T>) && */ \
        requires { T::represents_concept(CONCEPT_PROBE_NAME(Name){}); }; \
    CONCEPT_TYPECHECKER(Name)

#define CONCEPT_WITH_TYPE(Name) \
    CONCEPT(Name); \
    template<class T, class V> \
    concept Name##OfType = \
        Name<T> && ::nn::HasTypeOfType<T, V>; \
    template<class T, template<class>class C> \
    concept Name##OfTraits = \
        Name<T> && ::nn::HasTypeOfTraits<T, C>;

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
