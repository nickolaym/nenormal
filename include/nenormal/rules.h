#pragma once

#include "concepts.h"
#include "ct.h"
#include "str.h"
#include "substitute.h"
#include "tristate.h"
#include "inplace/inplace_tristate.h"
#include "compose.h"
#include "augmented.h"
#include "inplace/inplace_augmented.h"

#include <iostream>
#include <iomanip>

#include <cassert>

// rule function types

// input: just CtStr
template<class T> concept RuleInput = CtStr<T> || Augmented<T>;

template<class T> concept RuleFixedInput = std::same_as<T, std::string> || InplaceAugmented<T>;
template<class T> concept RuleInplaceArg = Inplace<T> && RuleFixedInput<typename T::type>;

// output:
// - fail (not matched)
// - intermediate string (matched, stops processing rules, continues machine loop)
// - final string (matched, stops processing rules, stops machine loop)

enum rule_state_t {
    regular_state,
    final_state,
};
template<class T> concept CtState = CtOf<T, rule_state_t>;

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

    constexpr decltype(auto) operator()(RuleFailedOutput auto const& nmy) const {
        RuleOutput auto out = rule{}(nmy.value);
        if constexpr (!out) {
            return nmy; // by ref
        } else {
            return out;
        }
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

// subroutine
template<Rule auto... ps> struct rules {
    REPRESENTS(Rule)

    constexpr rules() = default;
    constexpr rules(Rule auto...) {}

    constexpr decltype(auto) operator()(RuleFailedOutput auto const& nmy) const {
        return (nmy >> ... >> ps);
    }
    constexpr RuleOutput auto operator()(RuleInput auto t) const {
        return (not_matched_yet{t} >> ... >> ps);
    }
    constexpr inplace_state operator()(RuleFixedInput auto& t) const {
        inplace_argument<decltype(t)> a{t}; // reference to input
        (a || ... || a.updated_by(ps));
        return a.state;
    }
};
template<Rule auto... ps> constexpr rules<ps...> rules_v{};
// CTAD
template<Rule ...Ps> rules(Ps...) -> rules<Ps{}...>;

// empty rules do nothing

template<> struct rules<> {
    REPRESENTS(Rule)
    constexpr decltype(auto) operator()(RuleFailedOutput auto const& nmy) const {
        return nmy;
    }
    constexpr RuleOutput auto operator()(RuleInput auto t) const {
        return not_matched_yet{t};
    }
    constexpr auto operator()(RuleFixedInput auto& t) const {
        return k_not_matched_yet;
    }
};

// machine loop

template<Rule auto p> struct rule_loop_body {
    REPRESENTS(Rule)

    constexpr decltype(auto) operator()(RuleFailedOutput auto const& nmy) const {
        return p(nmy).commit();
    }
    constexpr RuleOutput auto operator()(RuleInput auto t) const {
        return p(t).commit();
    }
    constexpr auto operator()(RuleFixedInput auto& t) const {
        inplace_argument<decltype(t)> a{t}; // reference to input
        a.updated_by(p);
        a.commit();
        return a.state;
    }
};

template<Rule auto p> struct rule_loop {
    REPRESENTS(Rule)

    constexpr decltype(auto) operator()(RuleFailedOutput auto const& nmy) const {
        constexpr auto body = rule_loop_body<p>{};
        return nmy
            // unwrap the loop 10 times
            >> body >> body >> body >> body >> body
            >> body >> body >> body >> body >> body
            >> rule_loop{};
    }
    constexpr RuleOutput auto operator()(RuleInput auto t) const {
        constexpr auto body = rule_loop_body<p>{};
        return not_matched_yet{t}
            // unwrap the loop 10 times
            >> body >> body >> body >> body >> body
            >> body >> body >> body >> body >> body
            >> rule_loop{};
    }
    constexpr auto operator()(RuleFixedInput auto& t) const {
        constexpr auto body = rule_loop_body<p>{};
        inplace_argument<decltype(t)> a{t};
        size_t limit = 10000;
        while (!a) {
            if (--limit == 0) break;
            a.updated_by(body);
        }
        assert(limit); // probably infinite loop
        return a.state;
    }
};
template<Rule auto p> constexpr rule_loop<p> rule_loop_v{};

// machine is a function string -> string, without technical details
// about regular / final state.

template<Rule auto m> struct machine_fun {
    constexpr RuleInput auto operator()(RuleInput auto input) const {
        return m(input).value;
    }
    constexpr RuleFixedInput auto operator()(RuleFixedInput auto input) const {
        m(input);
        return input;
    }
};
template<Rule auto m> constexpr machine_fun<m> machine_fun_v{};

// control augmentation

template<Rule auto p> struct hidden_rule {
    REPRESENTS(Rule)
    constexpr decltype(auto) operator()(RuleFailedOutput auto const& nmy) const {
        RuleOutput auto out = not_matched_yet{extract_text(nmy.value)} >> p;
        if constexpr (!out) {
            return nmy;
        } else {
            return out.rebind(rebind_text(nmy.value, out.value));
        }
    }
    constexpr RuleOutput auto operator()(RuleInput auto t) const {
        RuleOutput auto out = p(extract_text(t));
        // combine old augmentation with new text,
        // then combine new kind of tristate result with new augmented
        return out.rebind(rebind_text(t, out.value));
    }
    constexpr auto operator()(RuleFixedInput auto& t) const {
        return p(inplace_extract_text(t));
    }
};
template<Rule auto p> constexpr hidden_rule<p> hidden_rule_v{};

template<Str auto name, Rule auto p> struct facade_rule {
    REPRESENTS(Rule)
    REPRESENTS(FacadeRule)

    friend std::ostream& operator << (std::ostream& os, facade_rule const& v) { return os << name.view(); }

    constexpr decltype(auto) operator()(RuleFailedOutput auto const& nmy) const {
        // host rebound arg in a scope-persistent storage
        RuleFailedOutput auto bare_nmy = not_matched_yet{extract_text(nmy.value)};
        RuleOutput auto const& out = p(bare_nmy);
        // combine old augmentation with new text,
        // then combine new kind of tristate result with new augmented
        if constexpr (!out) {
            return nmy;
        } else {
            return out.rebind(update_text(nmy.value, *this, out.value));
        }
    }
    constexpr RuleOutput auto operator()(RuleInput auto t) const {
        RuleOutput auto out = p(extract_text(t));
        // combine old augmentation with new text,
        // then combine new kind of tristate result with new augmented
        if constexpr (!out) {
            return out.rebind(rebind_text(t, out.value));
        } else {
            return out.rebind(update_text(t, *this, out.value));
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
CONCEPT(FacadeRule)

// useful macros

#define STR(s) (str{s}) // s##_ss
#define CTSTR(s) (ct<STR(s)>{}) // s##_cts
#define RULE(s, r) (rule_v<STR(s), STR(r), regular_state>)
#define FINAL_RULE(s, r) (rule_v<STR(s), STR(r), final_state>)
#define RULES(...) (rules_v<__VA_ARGS__>)
#define RULE_LOOP(r) (rule_loop_v<(r)>)
#define MACHINE_FROM_RULE(r) (machine_fun_v<(r)>)
#define MACHINE(r) MACHINE_FROM_RULE(RULE_LOOP(r))

#define HIDDEN_RULE(p) (hidden_rule_v<(p)>)
#define FACADE_RULE(name, p) (facade_rule_v<STR(name), (p)>)

// To hide a program from compiler output
#define NAMED_RULE(name, p) \
(struct name { \
    REPRESENTS(Rule) \
    static constexpr auto impl = (p); \
    constexpr decltype(auto) operator()(RuleFailedOutput auto const& nmy) const { return impl(nmy); } \
    constexpr RuleOutput auto operator()(RuleInput auto t) const { return impl(t); } \
    constexpr auto operator()(RuleFixedInput auto& t) const { return impl(t); } \
}){}
