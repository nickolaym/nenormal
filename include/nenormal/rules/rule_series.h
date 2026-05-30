#pragma once

#include "rule_concepts.h"
#include "empty_rule.h"
#include "../utility.h"

namespace nn {

// series of rules is a sequence of alternatives
// it takes first applicable rule and applies it

namespace rules_helper {

template<Rule P> struct found_rule {
    P p;
    constexpr decltype(auto) operator | (Rule auto&& q) const { return *this; }
};

template<RuleInput T> struct will_match_to {
    static constexpr Rule auto p = empty_rule{};
    constexpr auto operator | (Rule auto&& p) const {
        if constexpr (NotMatchedYet<decltype(p(std::declval<T>()))>)
            return *this;
        else
            return found_rule<decltype(p)>{FWD(p)};
    }
};

} // namespace rules_helper

template<Rule auto... ps> struct rules {
    REPRESENTS(Rule)

    constexpr rules() = default;
    constexpr rules(Rule auto...) {}

    constexpr RuleOutput decltype(auto) operator()(RuleInput auto&& nmy) const {
        // return (FWD(nmy) >> ... >> ps).commit_alts();
        return FWD(nmy) >> (rules_helper::will_match_to<decltype(nmy)>{} | ... | ps).p;
    }
    constexpr tristate_kind update(RuleFixedInput auto& t) const {
        inplace_argument<decltype(t)> a{t}; // reference to input
        (a || ... || a.updated_by(ps));
        return a.kind;
    }
};

template<Rule auto... ps> constexpr rules<ps...> rules_v{};

// empty rules do nothing

template<> struct rules<> : empty_rule {};

// CTAD - to write rules{p1, p2, ..., pn,} instead of rules<p1, p2, pn>
// (check out the trailing comma)

template<Rule ...Ps> rules(Ps...) -> rules<Ps{}...>;

} // namespace nn
