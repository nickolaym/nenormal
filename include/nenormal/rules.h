#pragma once

#include "concepts.h"
#include "ct.h"
#include "str.h"
#include "compose.h"
#include "augmented.h"

#include <algorithm>
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

template<class T> concept RuleOutput = Success<T> || Fail<T>;

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
        FailOrSubst auto res = try_substitute(extract_text(t));
        if constexpr (failed(res)) {
            return fail{};
        } else {
            RuleInput auto tt = update_text(t, rule{}, res);
            return success{tt, ct_state};
        }
    }

    static constexpr FailOrSubst auto try_substitute(CtStr auto t) {
        constexpr Str auto const& src = t.value;
        if constexpr (src.size() < s.size()) {
            return fail{};
        } else if constexpr (src == s) {
            return ct_replace;
        } else {
            constexpr auto fbegin = std::search(src.begin(), src.end(), s.begin(), s.end());
            if constexpr (fbegin == src.end()) {
                return fail{};
            } else {
                constexpr Str auto dst = // constexpr constant
                    [&]{
                        auto fend = fbegin + s.size();
                        constexpr auto len = src.size() - s.size() + r.size();
                        str<len + 1> dst; // not constant yet in this block
                        auto it = dst.begin();
                        it = std::copy(src.begin(), fbegin, it);
                        it = std::copy(r.begin(), r.end(), it);
                        it = std::copy(fend, src.end(), it);
                        *it = 0;
                        return dst;
                    }();
                return ct<dst>{};
            }
        }
    }
};

// subroutine

namespace rules_helper {
    // implements disjunction on input >> rule1 >> rule2 >> ...

    constexpr auto make_arg(RuleInput auto input) { return arg{input}; }
    constexpr auto make_fun(Rule auto rule) {
        // rule : input -> success | fail
        // fun  : arg<input> -> arg<input> | stop<success>
        return [rule]<RuleInput T>(arg<T> a) {
            RuleOutput auto r = rule(a.value);
            if constexpr (failed(r)) // not matched yet
                return a;            // continue...
            else
                return stop{r};      // success
        };
    }
    template<RuleInput T> constexpr fail take_res(arg<T> a) {
        // still not resolved
        return fail{};
    }
    template<RuleOutput T> constexpr T take_res(stop<T> a) {
        return a.value;
    }

} // namespace rules_helper

template<Rule auto... rs> struct rules {
    REPRESENTS(Rule)

    static constexpr auto the_chain = chain(rules_helper::make_fun(rs)...);

    constexpr RuleOutput auto operator()(RuleInput auto t) const {
        return rules_helper::take_res(the_chain(rules_helper::make_arg(t)));
    }
};

template<> struct rules<> {
    REPRESENTS(Rule)
    constexpr RuleOutput auto operator()(RuleInput auto t) const { return fail{}; }
};

// machine loop

namespace loop_helper {
    // implements endless loop on input >> rule >> rule >> ...

    constexpr auto make_arg(RuleInput auto input) { return arg{input}; }
    constexpr auto make_fun(Rule auto rule) {
        // rule : input -> success{output,(regular|final)} | fail
        // fun  : arg<input> -> arg<output> | stop<success>
        return [rule]<RuleInput T>(arg<T> a) {
            RuleOutput auto r = rule(a.value);
            if constexpr (failed(r))  // not matched anymore
                return stop{success{a.value, ct<regular_state>{}}}; // stop with previous value
            else if constexpr (r.state.value == final_state) // matched, final state
                return stop{r}; // stop with result
            else
                return arg{r.data}; // matched, regular state - continue with new value
        };
    }
    template<RuleInput T> constexpr fail take_res(arg<T> a) = delete; // never fails
    template<RuleOutput T> constexpr T take_res(stop<T> a) {
        return a.value;
    }

} // namespace loop_helper

template<Rule auto p, size_t const factor = 10> struct rule_loop {
    REPRESENTS(Rule)

    static constexpr auto the_loop = endless_loop{repeat<factor>(loop_helper::make_fun(p))};

    constexpr /* Success */ RuleOutput auto operator()(RuleInput auto t) const {
        return loop_helper::take_res(the_loop(loop_helper::make_arg(t)));
    }
};

// machine is a function string -> string, without technical details
// about regular / final state.

template<Rule auto m> struct machine_fun {
    constexpr RuleInput auto operator()(RuleInput auto input) const {
        return m(input).data;
    }
};

// control augmentation

template<Rule auto p> struct hidden_rule {
    REPRESENTS(Rule)
    constexpr RuleOutput auto operator()(RuleInput auto t) const {
        RuleOutput auto out = p(extract_text(t)); // success(str, state) or fail
        if constexpr (failed(out)) {
            return fail{};
        } else {
            return success{rebind_text(t, out.data), out.state};
        }
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
