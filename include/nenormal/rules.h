#pragma once

#include "concepts.h"
#include "ct.h"
#include "str.h"
#include "compose.h"

#include <iostream>
#include <iomanip>

template<CtStr T, class F> struct trace_input {
    REPRESENTS(TraceInput);
    static constexpr T value = T{};
    T text;
    F f;
};
CONCEPT(TraceInput);


// rule function types
// RuleInput -> RuleOutput

// input: just CtStr
template<class T> concept RuleInput = CtStr<T> || TraceInput<T>;

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

enum rule_state_t {
    regular_state,
    final_state,
};
template<class T> concept CtState = CtOf<T, rule_state_t>;

template<CtStr T, CtState S> struct success {
    REPRESENTS(Success);
    T text;
    S state;

    constexpr bool operator==(const success&) const = default;
    template<class T1, class S1>
    constexpr bool operator==(const success<T1, S1>&) const { return false; }
};
CONCEPT(Success);
constexpr bool finished(Success auto s) { return s.state.value == final_state; }

template<class T> concept RuleOutput = Success<T> || Fail<T>;

// single search-and-replace function
CONCEPT(Rule)

template<Str auto s, Str auto r, rule_state_t state> struct rule {
    REPRESENTS(Rule)
    static_assert(
        !(state == regular_state && s == r),
        "same search and replace is meaningless: either inaccessible or results in an endless loop");

    friend std::ostream& operator << (std::ostream& os, rule) {
        os << "rule(";
        os << std::quoted(s.value) << " -> " << std::quoted(r.value);
        if (state == final_state) os << ", final";
        os << ")";
        return os;
    }

    RuleOutput auto operator()(TraceInput auto t) const {
        constexpr auto dst = rule{}(t.value);
        if constexpr (!failed(dst)) {
            t.f(t.text, *this, dst.text);
        }
        return dst;
    }

    constexpr RuleOutput auto operator()(CtStr auto t) const {
        constexpr const Str auto& src = t.value;
        if constexpr (src.size() < s.size()) {
            return fail{};
        } else if constexpr (src == s) {
            return success{ct<r>{}, ct<state>{}};
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
                return success{ct<dst>{}, ct<state>{}};
            }
        }
    }
};

// subroutine

namespace rules_helper {
    // implements disjunction on text >> rule1 >> rule2 >> ...

    constexpr auto make_arg(RuleInput auto text) { return arg{text}; }
    constexpr auto make_fun(Rule auto rule) {
        // rule : CtStr -> success | fail
        // fun  : arg<CtStr> -> arg<CtStr> | stop<success>
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
    // implements endless loop on text >> rule >> rule >> ...

    constexpr auto make_arg(RuleInput auto text) { return arg{text}; }

    // constexpr auto next_arg(RuleInput auto src, CtStr auto dst) { return arg{dst}; }
    constexpr auto next_arg(CtStr auto src, CtStr auto dst) {
        return arg(dst);
    }
    constexpr auto next_arg(TraceInput auto src, CtStr auto dst) {
        return arg(trace_input{dst, src.f});
    }

    // constexpr auto stop_with_arg(RuleInput auto)
    constexpr CtStr auto src_text(CtStr auto text) { return text; }
    constexpr CtStr auto src_text(TraceInput auto t) { return t.text; }

    constexpr auto make_fun(Rule auto rule) {
        // rule : CtStr -> success{CtStr,(regular|final)} | fail
        // fun  : arg<RuleInput> -> arg<RuleInput> | stop<success>
        return [rule]<RuleInput T>(arg<T> a) {
            RuleOutput auto r = rule(a.value);
            if constexpr (failed(r))  // not matched anymore
                return stop{success{src_text(a.value), ct<regular_state>{}}}; // stop with previous value
            else if constexpr (finished(r)) // matched, final state
                return stop{r}; // stop with result
            else
                return next_arg(a.value, r.text); // matched, regular state - continue with new value
        };
    }
    template<RuleInput T> constexpr fail take_res(arg<T> a) = delete; // never fails
    template<RuleOutput T> constexpr T take_res(stop<T> a) {
        return a.value;
    }

} // namespace loop_helper

template<Rule auto p> struct rule_loop {
    REPRESENTS(Rule)

    static constexpr auto the_loop = endless_loop{repeat<10>(loop_helper::make_fun(p))};

    constexpr RuleOutput auto operator()(RuleInput auto t) const {
        return loop_helper::take_res(the_loop(loop_helper::make_arg(t)));
    }
};

// machine is a function string -> string, without technical details
// about regular / final state.

template<Rule auto m> struct machine_fun {
    constexpr CtStr auto operator()(RuleInput auto text) const {
        return m(text).text;
    }
};

// useful macros

#define STR(s) (str{s}) // s##_ss
#define CTSTR(s) (ct<STR(s)>{}) // s##_cts
#define RULE(s, r) (rule<STR(s), STR(r), regular_state>{})
#define FINAL_RULE(s, r) (rule<STR(s), STR(r), final_state>{})
#define RULES(...) rules<__VA_ARGS__>{}
#define RULE_LOOP(r) rule_loop<(r)>{}
#define MACHINE_FROM_RULE(r) machine_fun<(r)>{}
#define MACHINE(r) MACHINE_FROM_RULE(RULE_LOOP(r))

// To hide a program from compiler output
#define NAMED_RULE(name, p) \
(struct name { \
    REPRESENTS(Rule) \
    static constexpr auto impl = (p); \
    constexpr RuleOutput auto operator()(RuleInput auto t) const { return impl(t); } \
}){}
