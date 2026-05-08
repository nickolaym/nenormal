#pragma once

#include "rule_concepts.h"
#include "../utility.h"
#include "../substitute.h"


#include <iostream>
#include <iomanip>

// single rule is a primary element of the NAM.
// it matches and modifies the text state of the NAM-machine.

// a rule is parametrized with
// - search string "s"
// - replace string "r"
// - kind: regular or final

enum rule_state_t {
    regular_state,
    final_state,
};
template<class T> concept CtState = CtOf<T, rule_state_t>;

// single search-and-replace function

template<Str auto s, Str auto r, rule_state_t state> struct rule {
    REPRESENTS(Rule)
    REPRESENTS(SingleRule)
    static_assert(
        !(state == regular_state && s == r),
        "same search and replace is meaningless: either inaccessible or results in an endless loop");

    static constexpr auto ct_search = ct<s>{};
    static constexpr auto ct_replace = ct<r>{};
    static constexpr auto ct_state = ct<state>{};

    friend std::ostream& operator << (std::ostream& os, rule const& v) {
        os
            << (state == regular_state ? "rule" : "final_rule")
            << "("
            << std::quoted(s.view())
            << " -> "
            << std::quoted(r.view())
            << ")";
        return os;
    }

    constexpr decltype(auto) operator()(RuleNotMatchedYetInputRef auto&& nmy) const {
        MaybeCtStr auto mb = try_substitute(ct_search, ct_replace, extract_text(nmy.value));
        if constexpr (!mb) {
            return FWD(nmy);
        } else {
            if constexpr (state == regular_state) {
                return matched_regular{update_text(nmy.value, rule{}, mb.value)};
            } else {
                return matched_final{update_text(nmy.value, rule{}, mb.value)};
            }
        }
    }
    constexpr RuleOutput auto operator()(RuleInputRef auto&& t) const {
        MaybeCtStr auto mb = try_substitute(ct_search, ct_replace, extract_text(t));
        if constexpr (!mb) {
            // failed
            return not_matched_yet{FWD(t)};
        } else {
            if constexpr (state == regular_state) {
                return matched_regular{update_text(t, rule{}, mb.value)};
            } else {
                return matched_final{update_text(t, rule{}, mb.value)};
            }
        }
    }

    constexpr inplace_state operator()(RuleFixedInput auto& t) const {
        if (!try_substitute_inplace(ct_search, ct_replace, inplace_extract_text(t))) {
            return k_not_matched_yet;
        } else {
            inplace_update_text(t, rule{});
            if (state == regular_state) {
                return k_matched_regular;
            } else {
                return k_matched_final;
            }
        }
    }
};

template<Str auto s, Str auto r, rule_state_t state> constexpr rule<s, r, state> rule_v{};
