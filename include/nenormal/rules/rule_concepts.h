#pragma once

#include "arg_types.h"

namespace nn {

CONCEPT(Rule)

template<class T> concept RuleRequires =
    std::is_default_constructible_v<T> &&
    std::is_copy_constructible_v<T> &&
    requires (T p) {
        // obsolete form
        { p(CTSTR("")) } -> RuleOutput;
        { p(augmented_text{CTSTR(""),empty{}}) } -> RuleOutput;
        // modern form
        { p(not_matched_yet{CTSTR("")}) } -> RuleOutput;
        { p(not_matched_yet{augmented_text{CTSTR(""),empty{}}}) } -> RuleOutput;
        // inplace
        { p(std::string{}) } -> std::same_as<inplace_state>;
        { p(inplace_augmented_text{std::string{}, inplace_empty{}}) } -> std::same_as<inplace_state>;
    };

// particular rule concepts (for user-defined augmentation)

CONCEPT(SingleRule)
CONCEPT(FacadeRule)

} // namespace nn
