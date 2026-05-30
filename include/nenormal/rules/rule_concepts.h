#pragma once

#include "arg_types.h"

namespace nn {

CONCEPT(Rule)

template<class T> concept RuleRequires =
    std::is_default_constructible_v<T> &&
    std::is_copy_constructible_v<T> &&
    requires (T p) {
        { p(not_matched_yet{CTSTR("")}) } -> RuleOutput;
        { p(not_matched_yet{augmented_text{CTSTR(""),empty{}}}) } -> RuleOutput;
        // inplace
        { p.update(std::string{}) } -> std::same_as<tristate_kind>;
        { p.update(inplace_augmented_text{std::string{}, inplace_empty{}}) } -> std::same_as<tristate_kind>;
    } && !requires(T p) {
        { p(CTSTR("")) } -> RuleOutput;
    } && !requires(T p) {
        { p(augmented_text{CTSTR(""),empty{}}) } -> RuleOutput;
    };

// particular rule concepts (for user-defined augmentation)

CONCEPT(SingleRule)
CONCEPT(FacadeRule)

CONCEPT(Machine)

template<class T> concept MachineRequires =
    std::is_default_constructible_v<T> &&
    std::is_copy_constructible_v<T> &&
    requires (T m) {
        { m(CTSTR("")) } -> CtStr;
        { m(augmented_text{CTSTR(""), empty{}}) } -> Augmented;
        { m(std::string{}) } -> std::same_as<std::string>;
        { m(inplace_augmented_text{std::string{}, inplace_empty{}}) } -> InplaceAugmented;
    };

} // namespace nn
