#pragma once

#include "rule_concepts.h"
#include "../utility.h"
#include "../scope_exit.h"
#include <limits>

namespace nn {

// rule loop repeatedly applies its body until reached some final state.
// to provide that, special wrapper rule_loop_body translates result of nested rule:
// - not_matched_yet - to matched_final_halted (to prevent endless loop)
// - matched_regular - to not_matched_yet (to retry with new text)

#define DEBUG_CALL_PAIR(name_cts) \
    auto debug_callback = get_debug_callback(nmy.value); \
    debug_callback(concat_ctstr(CTSTR("<"), name_cts, CTSTR(">")).value.view()); \
    SCOPE_EXIT() { \
    debug_callback(concat_ctstr(CTSTR("</"), name_cts, CTSTR(">")).value.view()); \
    };

constexpr size_t rule_loop_limit_v = 5000;
constexpr size_t rule_loop_unlimited_v = ::std::numeric_limits<size_t>::max();

namespace rule_loop_helpers_ns {

template<Rule auto p> struct rule_loop_body {
    REPRESENTS(Rule)

    constexpr RuleOutput auto operator()(RuleInput auto&& nmy) const {
        DEBUG_CALL_PAIR(CTSTR("body"));
        return p(FWD(nmy)).commit_loop();
    }
    constexpr tristate_kind update(RuleFixedInput auto& t) const {
        inplace_argument<decltype(t)> a{t}; // reference to input
        a.updated_by(p);
        a.commit();
        return a.kind;
    }
};

// rule_loop unrolls the loop over recursion, to reduce its depth.
// in case of homogenous in-out type unrolling is not required.

// Heapifying rule loop is an idea how to reduce depth of recursion.
//
// Naive loop is
// loop p = \x -> x >> p >> (loop p)
// Recursion depth D after T times of iteration is, obviously, D = T.
//
// Linear loop is
// loop p = \x -> x >> (multiply u p) >> (loop p)
// D = T/u
// where u is an unrolling factor.
//
// Heap-like loop is
// loop p = \x -> x >> (multiply u p) >> (loop (multiply m p))
// D ~ log_m(T/u)
//
// The cost of this solution is:
// - complexity
// - extra calls of move constructors
//
// Experiments show that linear loop is cheap enough,
// so default parameters are:
// - unroll = 50
// - multiply = 1

constexpr auto chain(NotMatchedYet auto&& nmy)
{ return FWD(nmy); }

constexpr auto repeat(NotMatchedYet auto&& nmy, Rule auto&& p, CtSize auto ctn)
requires (ctn.value > 0)
{
    if constexpr (ctn.value == 1 || Matched<decltype(p(FWD(nmy)))>)
        return p(FWD(nmy));
    else
        return repeat(p(FWD(nmy)), FWD(p), ct_size_v<ctn.value - 1>);
}

constexpr auto chain(NotMatchedYet auto&& nmy, Rule auto&& p, Rule auto&&... ps)
{
    // constexpr size_t N = (sizeof...(ps))+1;
    // DEBUG_CALL_PAIR(concat_ctstr(CTSTR("chain:"), size_to_ctstr(ct_size_v<N>)));

    if constexpr ((sizeof...(ps) == 0) || Matched<decltype(p(FWD(nmy)))>)
        return p(FWD(nmy));
    else
        return chain(p(FWD(nmy)), FWD(ps)...);
}

template<Rule auto body, size_t N>
struct multiply_body {
    REPRESENTS(Rule)
    constexpr RuleOutput auto operator()(RuleInput auto&& nmy) const {
        DEBUG_CALL_PAIR(concat_ctstr(CTSTR("mult:"), size_to_ctstr(ct_size_v<N>)));
        if constexpr (N == 0)
            return FWD(nmy);
        else if constexpr (N == 1)
            return body(FWD(nmy));
        else if constexpr (N <= 10)
            return repeat(FWD(nmy), body, ct_size_v<N>);
        else
            return chain(FWD(nmy),
                multiply_body<body, 10>{},
                multiply_body<body, N - 10>{});
    }
};

// L - limit
// D - delta (first linear part)
// M - multiplier (second recursive part)
template<Rule auto body, size_t Limit, size_t Unroll, size_t Multiply>
requires (Unroll > 0) && (Multiply > 0)
struct repeat_body {
    REPRESENTS(Rule)
    constexpr RuleOutput auto operator()(RuleInput auto&& nmy) const {
        DEBUG_CALL_PAIR(concat_ctstr(CTSTR("repeat:"), size_to_ctstr(ct_size_v<Limit>)));
        if constexpr (Limit == 0) {
            return FWD(nmy);
        } else if constexpr (Limit <= Unroll) {
            return chain(FWD(nmy),
                multiply_body<body, Limit>{});
        } else if constexpr (Multiply == 1) {
            constexpr auto LimitRest = (Limit == rule_loop_unlimited_v) ? Limit : Limit - Unroll;
            return chain(FWD(nmy),
                multiply_body<body, Unroll>{},
                repeat_body<body, LimitRest, Unroll, Multiply>{});
        } else if constexpr (Limit == rule_loop_unlimited_v) {
            return chain(FWD(nmy),
                multiply_body<body, Unroll>{},
                repeat_body<multiply_body<body, Multiply>{}, Limit, Unroll, Multiply>{});
        } else {
            constexpr auto R = (Limit - Unroll) % Multiply;
            constexpr auto UnrollRound = Unroll + R;
            constexpr auto LimitRest = Limit - UnrollRound; // Lrest % Multiply == 0
            constexpr auto LimitRestM = LimitRest / Multiply;
            return chain(FWD(nmy),
                multiply_body<body, UnrollRound>{},
                repeat_body<multiply_body<body, Multiply>{}, LimitRestM, Unroll, Multiply>{});
        }
    }
};

} // namespace rule_loop_helpers_ns

template<Rule auto p, size_t Limit = rule_loop_limit_v> struct rule_loop {
    REPRESENTS(Rule)

    static constexpr size_t Unroll = 50;
    static constexpr size_t Multiply = 1;

    constexpr RuleOutput auto operator()(RuleInput auto&& nmy) const {
        DEBUG_CALL_PAIR(concat_ctstr(CTSTR("loop:"), size_to_ctstr(ct_size_v<Limit>)));
        constexpr auto body = rule_loop_helpers_ns::rule_loop_body<p>{};
        return FWD(nmy) >> rule_loop_helpers_ns::repeat_body<body, Limit, Unroll, Multiply>{};
    }
    constexpr tristate_kind update(RuleFixedInput auto& t) const {
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
