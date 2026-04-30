#pragma once

#include "concepts.h"
#include "ct.h"
#include "str.h"
#include "substitute.h"
#include "tristate.h"
#include "compose.h"
#include "augmented.h"

#include <iostream>
#include <iomanip>

// rule function types

// input: just CtStr
template<class T> concept RuleInput = CtStr<T> || Augmented<T>;

// output:
// - fail (not matched)
// - intermediate string (matched, stops processing rules, continues machine loop)
// - final string (matched, stops processing rules, stops machine loop)

struct fail {
    friend std::ostream& operator << (std::ostream& os, fail const& v) { os << "fail"; return os; }

    constexpr bool operator == (const fail&) const = default;
    constexpr bool operator == (const auto&) const { return false; }
};
constexpr bool failed(const auto& a) { return fail{} == a; }
template<class T> concept Fail = std::same_as<T, fail>;

// opposite to fail
template<class T> concept FailOrSubst = Fail<T> || CtStr<T>;

enum rule_state_t {
    regular_state,
    final_state,
};
template<class T> concept CtState = CtOf<T, rule_state_t>;

// successful result will be passed to the next loop.
// that's why type T is a kind of RuleInput
template<RuleInput T, CtState S> struct success {
    REPRESENTS(Success);
    T data;
    S state;  // TODO make it static!

    constexpr bool operator==(const success&) const = default;
    template<class T1, class S1>
    constexpr bool operator==(const success<T1, S1>&) const { return false; }
};
CONCEPT(Success);
constexpr bool finished(Success auto s) { return s.state.value == final_state; }

template<class T> concept RuleOutput = Tristate<T> && RuleInput<typename T::type>;
template<class T> concept RuleMatchedOutput = RuleOutput<T> && !NotMatchedYet<T>;
template<class T> concept RuleFailedOutput = RuleOutput<T> && NotMatchedYet<T>;

// single search-and-replace function
CONCEPT(Rule)
CONCEPT(SingleRule)

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

    constexpr RuleOutput auto operator()(RuleInput auto t) const {
        MaybeCtStr auto mb = try_substitute(ct_search, ct_replace, extract_text(t));
        if constexpr (!mb) {
            // failed
            return not_matched_yet{t};
        } else {
            RuleInput auto u = update_text(t, rule{}, mb.value);
            if constexpr (state == regular_state) {
                return matched_regular{u};
            } else {
                return matched_final{u};
            }
        }
    }
};

// subroutine

template<Rule auto... ps> struct rules {
    REPRESENTS(Rule)

    constexpr RuleOutput auto operator()(RuleInput auto t) const {
        return (not_matched_yet{t} >> ... >> ps);
    }
};

// empty rules do nothing

template<> struct rules<> {
    REPRESENTS(Rule)
    constexpr RuleOutput auto operator()(RuleInput auto t) const { return not_matched_yet{t}; }
};

// machine loop

template<Rule auto p> struct rule_loop_body {
    REPRESENTS(Rule)

    constexpr RuleOutput auto operator()(RuleInput auto t) const {
        return p(t).commit();
    }
};

template<Rule auto p> struct rule_loop {
    REPRESENTS(Rule)

    constexpr RuleOutput auto operator()(RuleInput auto t) const {
        constexpr auto body = rule_loop_body<p>{};
        return not_matched_yet{t}
            // unwrap the loop 10 times
            >> body >> body >> body >> body >> body
            >> body >> body >> body >> body >> body
            >> rule_loop{};
    }
};

// machine is a function string -> string, without technical details
// about regular / final state.

template<Rule auto m> struct machine_fun {
    constexpr RuleInput auto operator()(RuleInput auto input) const {
        return m(input).value;
    }
};

// control augmentation

template<Rule auto p> struct hidden_rule {
    REPRESENTS(Rule)
    constexpr RuleOutput auto operator()(RuleInput auto t) const {
        RuleOutput auto out = p(extract_text(t));
        // combine old augmentation with new text,
        // then combine new kind of tristate result with new augmented
        return out.rebind(rebind_text(t, out.value));
    }
};

// useful macros

#define STR(s) (str{s}) // s##_ss
#define CTSTR(s) (ct<STR(s)>{}) // s##_cts
#define RULE(s, r) (rule<STR(s), STR(r), regular_state>{})
#define FINAL_RULE(s, r) (rule<STR(s), STR(r), final_state>{})
#define RULES(...) (rules<__VA_ARGS__>{})
#define RULE_LOOP(r) (rule_loop<(r)>{})
#define MACHINE_FROM_RULE(r) (machine_fun<(r)>{})
#define MACHINE(r) MACHINE_FROM_RULE(RULE_LOOP(r))

#define HIDDEN_RULE(p) (hidden_rule<(p)>{})

// To hide a program from compiler output
#define NAMED_RULE(name, p) \
(struct name { \
    REPRESENTS(Rule) \
    static constexpr auto impl = (p); \
    constexpr RuleOutput auto operator()(RuleInput auto t) const { return impl(t); } \
}){}
