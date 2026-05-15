#pragma once

#include "rule_concepts.h"
#include "../utility.h"

namespace nn {

// rule loop repeatedly applies its body until reached some final state.
// to provide that, special wrapper rule_loop_body translates result of nested rule:
// - not_matched_yet - to matched_final_halted (to prevent endless loop)
// - matched_regular - to not_matched_yet (to retry with new text)

template<Rule auto p> struct rule_loop_body {
    REPRESENTS(Rule)

    constexpr RuleOutput decltype(auto) operator()(RuleInput auto&& nmy) const {
        return p(FWD(nmy)).commit_loop();
    }
    constexpr tristate_kind operator()(RuleFixedInput auto& t) const {
        inplace_argument<decltype(t)> a{t}; // reference to input
        a.updated_by(p);
        a.commit();
        return a.kind;
    }
};

// rule_loop unrolls the loop over recursion, to reduce its depth.
// in case of homogenous in-out type unrolling is not required.

template<Rule auto p> struct rule_loop {
    REPRESENTS(Rule)

    constexpr RuleFinalOutput auto operator()(RuleInput auto&& nmy) const {
        constexpr auto body = rule_loop_body<p>{};
        return (FWD(nmy)
            // unwrap the loop 10 times
            >> body >> body >> body >> body >> body
            >> body >> body >> body >> body >> body
            >> rule_loop{}
        ).commit_alts();
    }
    constexpr tristate_kind operator()(RuleFixedInput auto& t) const {
        constexpr auto body = rule_loop_body<p>{};
        inplace_argument<decltype(t)> a{t};
        size_t limit = 10000;
        while (!a) {
            if (--limit == 0) throw std::runtime_error("the loop runs too long");
            a.updated_by(body);
        }
        return a.kind;
    }
};

template<Rule auto p> constexpr rule_loop<p> rule_loop_v{};

} // namespace nn
