#pragma once

#include "rule_concepts.h"

namespace nn {

// hidden_rule wrapper hides nested rule from augmentation.
// nested rule deals with bare string, and the result mixes with old augmentation data.
// this allows hide intermediate iterations of given NAM program off the view.

template<Rule auto p> struct hidden_rule {
    REPRESENTS(Rule)

    constexpr RuleOutput decltype(auto) operator()(RuleInput auto&& nmy) const {
        RuleInput auto bare_nmy = not_matched_yet{extract_text(nmy.value)};
        RuleOutput auto bare_out = p(bare_nmy);
        if constexpr (!bare_out.is_matched) {
            return FWD(nmy);
        } else {
            return bare_out.rebind(rebind_text(FWD(nmy).value, bare_out.value));
        }
    }
    constexpr auto operator()(RuleFixedInput auto& t) const {
        return p(inplace_extract_text(t));
    }
};

template<Rule auto p> constexpr hidden_rule<p> hidden_rule_v{};

} // namespace nn
