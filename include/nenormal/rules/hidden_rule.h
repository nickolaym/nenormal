#pragma once

#include "rule_concepts.h"

namespace nn {

// hidden_rule wrapper hides nested rule from augmentation.
// nested rule deals with bare string, and the result mixes with old augmentation data.
// this allows hide intermediate iterations of given NAM program off the view.

template<Rule auto p> struct hidden_rule {
    REPRESENTS(Rule)

    constexpr RuleOutput decltype(auto) operator()(RuleNotMatchedYetInput auto&& nmy) const {
        RuleOutput auto out = not_matched_yet{extract_text(nmy.value)} >> p;
        if constexpr (!out.is_matched) {
            return FWD(nmy);
        } else {
            return out.rebind(rebind_text(FWD(nmy).value, out.value));
        }
    }
    constexpr auto operator()(RuleFixedInput auto& t) const {
        return p(inplace_extract_text(t));
    }
};

template<Rule auto p> constexpr hidden_rule<p> hidden_rule_v{};

} // namespace nn
