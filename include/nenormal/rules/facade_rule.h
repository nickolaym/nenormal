#pragma once

#include "rule_concepts.h"
#include <iostream>

namespace nn {

// facade_rule wrapper masks its nested rule for the augmentation callback.
// like in hidden_rule, the nested rule deals with bare string.
// but in case of success, facade rule triggers augmentation with itself.
// this allows give common name for a group of rules.

template<Str auto name, Rule auto p> struct facade_rule {
    REPRESENTS(Rule)
    REPRESENTS(FacadeRule)

    friend std::ostream& operator << (std::ostream& os, facade_rule const& v) { return os << name.view(); }

    constexpr decltype(auto) operator()(RuleNotMatchedYetInputRef auto&& nmy) const {
        // host rebound arg in a scope-persistent storage
        RuleFailedOutput auto bare_nmy = not_matched_yet{extract_text(nmy.value)};
        RuleOutput auto const& out = p(bare_nmy);
        // combine old augmentation with new text,
        // then combine new kind of tristate result with new augmented
        if constexpr (!out.is_matched) {
            return FWD(nmy);
        } else {
            return out.rebind(update_text(FWD(nmy).value, *this, out.value));
        }
    }
    constexpr RuleOutput auto operator()(RuleInputRef auto&& t) const {
        RuleOutput auto out = p(extract_text(t));
        // combine old augmentation with new text,
        // then combine new kind of tristate result with new augmented
        if constexpr (!out.is_matched) {
            return out.rebind(rebind_text(FWD(t), out.value));
        } else {
            return out.rebind(update_text(FWD(t), *this, out.value));
        }
    }
    constexpr auto operator()(RuleFixedInput auto& t) const {
        auto res = p(inplace_extract_text(t));
        if (res != k_not_matched_yet) {
            inplace_update_text(t, *this);
        }
        return res;
    }
};

template<Str auto name, Rule auto p> constexpr facade_rule<name, p> facade_rule_v{};

} // namespace nn
