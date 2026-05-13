#pragma once

#include "rule_concepts.h"
#include "../utility.h"

namespace nn {

// series of rules is a sequence of alternatives
// it takes first applicable rule and applies it

// empty sequence is a rule that never matches.
// it may be useful for aestetic and debug purposes.

template<Rule auto... ps> struct rules {
    REPRESENTS(Rule)

    constexpr rules() = default;
    constexpr rules(Rule auto...) {}

    constexpr RuleOutput decltype(auto) operator()(RuleNotMatchedYetInput auto&& nmy) const {
        return (FWD(nmy) >> ... >> ps).commit_alts();
    }
    constexpr RuleOutput auto operator()(RuleInput auto&& t) const {
        return (not_matched_yet{FWD(t)} >> ... >> ps);
    }
    constexpr tristate_kind operator()(RuleFixedInput auto& t) const {
        inplace_argument<decltype(t)> a{t}; // reference to input
        (a || ... || a.updated_by(ps));
        return a.kind;
    }
};

template<Rule auto... ps> constexpr rules<ps...> rules_v{};

// empty rules do nothing

template<> struct rules<> {
    REPRESENTS(Rule)
    constexpr RuleOutput decltype(auto) operator()(RuleNotMatchedYetInput auto&& nmy) const {
        return FWD(nmy);
    }
    constexpr RuleOutput auto operator()(RuleInput auto&& t) const {
        return not_matched_yet{FWD(t)};
    }
    constexpr auto operator()(RuleFixedInput auto& t) const {
        return tristate_kind::not_matched_yet;
    }
};

// CTAD - to write rules{p1, p2, ..., pn,} instead of rules<p1, p2, pn>
// (check out the trailing comma)

template<Rule ...Ps> rules(Ps...) -> rules<Ps{}...>;

} // namespace nn
