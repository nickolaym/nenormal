#pragma once

#include "rule_concepts.h"
#include "../utility.h"

namespace nn {

// empty rule is a rule that never matches.
// it may be useful for aestetic and debug purposes.

struct empty_rule {
    REPRESENTS(Rule)
    constexpr RuleOutput decltype(auto) operator()(RuleInput auto&& nmy) const {
        return FWD(nmy);
    }
    constexpr auto update(RuleFixedInput auto& t) const {
        return tristate_kind::not_matched_yet;
    }
};

} // namespace nn
