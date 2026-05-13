#pragma once

#include "rule_concepts.h"
#include "../utility.h"
#include "../substitute.h"

#include <iostream>
#include <iomanip>

namespace nn {

// single rule is a primary element of the NAM.
// it matches and modifies the text k of the NAM-machine.

// a rule is parametrized with
// - search string "s"
// - replace string "r"
// - kind: regular or final

enum class rule_kind {
    regular,
    final,
};
template<class T> concept CtRuleKind = CtOfType<T, rule_kind>;

// single search-and-replace function

template<Str auto s, Str auto r, rule_kind k> struct rule {
    REPRESENTS(Rule)
    REPRESENTS(SingleRule)
    static_assert(
        !(k == rule_kind::regular && s == r),
        "same search and replace is meaningless: either inaccessible or results in an endless loop");

    static constexpr auto ct_search = ct<s>{};
    static constexpr auto ct_replace = ct<r>{};
    static constexpr auto ct_kind = ct<k>{};

    friend std::ostream& operator << (std::ostream& os, rule const& v) {
        os
            << "("
            << std::quoted(s.view())
            << (k == rule_kind::regular ? " -> " : " ->. ")
            << std::quoted(r.view())
            << ")";
        return os;
    }

    constexpr decltype(auto) operator()(RuleNotMatchedYetInputRef auto&& nmy) const {
        MaybeCtStr auto mb = try_substitute(ct_search, ct_replace, extract_text(nmy.value));
        if constexpr (!mb) {
            return FWD(nmy);
        } else {
            if constexpr (k == rule_kind::regular) {
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
            if constexpr (k == rule_kind::regular) {
                return matched_regular{update_text(t, rule{}, mb.value)};
            } else {
                return matched_final{update_text(t, rule{}, mb.value)};
            }
        }
    }

    constexpr tristate_kind operator()(RuleFixedInput auto& t) const {
        if (!try_substitute_inplace(ct_search, ct_replace, inplace_extract_text(t))) {
            return tristate_kind::not_matched_yet;
        } else {
            inplace_update_text(t, rule{});
            if (k == rule_kind::regular) {
                return tristate_kind::matched_regular;
            } else {
                return tristate_kind::matched_final;
            }
        }
    }
};

template<Str auto s, Str auto r, rule_kind k> constexpr rule<s, r, k> rule_v{};

} // namespace nn
