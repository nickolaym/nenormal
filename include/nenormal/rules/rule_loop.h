#pragma once

#include "rule_concepts.h"
#include "../utility.h"

namespace nn {

// rule loop repeatedly applies its body until reached some final state.
// to provide that, special wrapper rule_loop_body translates result of nested rule:
// - not_matched_yet - to matched_final_halted (to prevent endless loop)
// - matched_regular - to not_matched_yet (to retry with new text)

namespace rule_loop_helpers_ns {

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

template<Rule auto body, size_t N>
requires (N > 0) && (N <= 10)
struct multiply_body {
    REPRESENTS(Rule)
    constexpr RuleOutput auto operator()(RuleInput auto&& nmy) const {
        if constexpr (N == 1)
            return FWD(nmy)
                >> body;
        else if constexpr (N == 2)
            return FWD(nmy)
                >> body >> body;
        else if constexpr (N == 3)
            return FWD(nmy)
                >> body >> body >> body;
        else if constexpr (N == 4)
            return FWD(nmy)
                >> body >> body >> body >> body;
        else if constexpr (N == 5)
            return FWD(nmy)
                >> body >> body >> body >> body >> body;
        else if constexpr (N == 6)
            return FWD(nmy)
                >> body >> body >> body >> body >> body
                >> body;
        else if constexpr (N == 7)
            return FWD(nmy)
                >> body >> body >> body >> body >> body
                >> body >> body;
        else if constexpr (N == 8)
            return FWD(nmy)
                >> body >> body >> body >> body >> body
                >> body >> body >> body;
        else if constexpr (N == 9)
            return FWD(nmy)
                >> body >> body >> body >> body >> body
                >> body >> body >> body >> body;
        else if constexpr (N == 10)
            return FWD(nmy)
                >> body >> body >> body >> body >> body
                >> body >> body >> body >> body >> body;
        else
            static_assert(false, "not supported arbitrary N");
    }
};

template<Rule auto body, size_t L>
struct repeat_body {
    REPRESENTS(Rule)
    constexpr RuleOutput auto operator()(RuleInput auto&& nmy) const {
        if constexpr (L == 0)
            return FWD(nmy);
        else if constexpr (L <= 10)
            return FWD(nmy)
                >> multiply_body<body, L>{};
        else if constexpr (L % 2 == 0)
            return FWD(nmy)
                >> multiply_body<body, 10>{}
                >> repeat_body< multiply_body<body, 2>{}, (L - 10) / 2>{};
        else
            return FWD(nmy)
                >> multiply_body<body, 9>{}
                >> repeat_body< multiply_body<body, 2>{}, (L - 9) / 2>{}; 
    }
};

} // namespace rule_loop_helpers_ns

constexpr size_t rule_loop_limit_v = 5000;

template<Rule auto p, size_t Limit = rule_loop_limit_v> struct rule_loop {
    REPRESENTS(Rule)

    constexpr RuleOutput auto operator()(RuleInput auto&& nmy) const {
        constexpr auto body = rule_loop_helpers_ns::rule_loop_body<p>{};
        return FWD(nmy) >> rule_loop_helpers_ns::repeat_body<body, Limit>{};
    }
    constexpr tristate_kind operator()(RuleFixedInput auto& t) const {
        constexpr auto body = rule_loop_helpers_ns::rule_loop_body<p>{};
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

template<Rule auto p, size_t L = rule_loop_limit_v> constexpr rule_loop<p, L> rule_loop_v{};

} // namespace nn
