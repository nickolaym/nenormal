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

template<Rule auto p, size_t Limit = 5000> struct rule_loop {
    REPRESENTS(Rule)

    constexpr RuleOutput auto operator()(RuleInput auto&& nmy) const {
        constexpr auto body = rule_loop_body<p>{};

        if constexpr (Limit == 0)
            return FWD(nmy); // by value
        else if constexpr (Limit == 1)
            return FWD(nmy)
                >> body;
        else if constexpr (Limit == 2)
            return FWD(nmy)
                >> body >> body;
        else if constexpr (Limit == 3)
            return FWD(nmy)
                >> body >> body >> body;
        else if constexpr (Limit == 4)
            return FWD(nmy)
                >> body >> body >> body >> body;
        else if constexpr (Limit == 5)
            return FWD(nmy)
                >> body >> body >> body >> body >> body;
        else if constexpr (Limit == 6)
            return FWD(nmy)
                >> body >> body >> body >> body >> body
                >> body;
        else if constexpr (Limit == 7)
            return FWD(nmy)
                >> body >> body >> body >> body >> body
                >> body >> body;
        else if constexpr (Limit == 8)
            return FWD(nmy)
                >> body >> body >> body >> body >> body
                >> body >> body >> body;
        else if constexpr (Limit == 9)
            return FWD(nmy)
                >> body >> body >> body >> body >> body
                >> body >> body >> body >> body;
        else if constexpr (Limit == 10)
            return FWD(nmy)
                >> body >> body >> body >> body >> body
                >> body >> body >> body >> body >> body;
        else
            return FWD(nmy)
                >> body >> body >> body >> body >> body
                >> body >> body >> body >> body >> body
                >> rule_loop<p, Limit - 10>{};
    }
    constexpr tristate_kind operator()(RuleFixedInput auto& t) const {
        constexpr auto body = rule_loop_body<p>{};
        inplace_argument<decltype(t)> a{t};
        size_t limit = Limit;
        while (!a) {
            if (limit == 0) break;
            --limit;
            a.updated_by(body);
        }
        return a.kind;
    }
};

template<Rule auto p> constexpr rule_loop<p> rule_loop_v{};

} // namespace nn
