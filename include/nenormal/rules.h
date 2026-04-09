#pragma once

#include "concepts.h"
#include "ct.h"
#include "str.h"
#include "compose.h"

#include <iostream>
#include <iomanip>

// rule function types

struct fail {
    friend std::ostream& operator << (std::ostream& os, fail const& v) { os << "fail"; return os; }

    constexpr bool operator == (const fail&) const = default;
    constexpr bool operator == (const auto&) const { return false; }
};
constexpr bool failed(const auto& a) { return fail{} == a; }

template<class T> concept Fail = std::same_as<T, fail>;
template<class T> concept StrOrFail = Str<T> || Fail<T>;


// single search-and-replace function
CONCEPT(Rule)

template<Str auto s, Str auto r> struct rule {
    REPRESENTS(Rule)
    static_assert(s != str{""}, "empty search string results in an endless loop");
    static_assert(s != r, "same search and replace results in an endless loop");
    friend std::ostream& operator << (std::ostream& os, rule const& v) {
        os << "rule(" << v.s << " -> " << v.r << ")";
        return os;
    }

    constexpr auto operator()(CtStr auto t) const {
        constexpr auto& src = t.value;
        if constexpr (src.size() < s.size()) {
            return fail{};
        } else if constexpr (src == s) {
            return r;
        } else {
            constexpr auto fbegin = std::search(src.begin(), src.end(), s.begin(), s.end());
            if constexpr (fbegin == src.end()) {
                return fail{};
            } else {
                return ct<
                    [&]{
                        auto fend = fbegin + s.size();
                        constexpr auto len = src.size() - s.size() + r.size();
                        str<len + 1> dst;
                        auto it = dst.begin();
                        it = std::copy(src.begin(), fbegin, it);
                        it = std::copy(r.begin(), r.end(), it);
                        it = std::copy(fend, src.end(), it);
                        *it = 0;
                        return dst;
                    }()
                >{};
            }
        }
    }
};

// subroutine

namespace rules_helper {
    // implements disjunction on text >> rule1 >> rule2 >> ...

    constexpr auto make_arg(CtStr auto text) { return arg{text}; }
    constexpr auto make_fun(Rule auto rule) {
        // rule : CtStr -> CtStr | fail
        // fun  : arg<CtStr> -> arg<CtStr> | stop<CtStr>
        return [rule]<CtStr T>(arg<T> a) {
            auto r = rule(a.value);
            if constexpr (failed(r)) // not matched yet
                return a;            // continue...
            else
                return stop{r};      // success
        };
    }
    template<CtStr T> constexpr fail make_res(arg<T> a) {
        // still not resolved
        return fail{};
    }
    template<CtStr T> constexpr T make_res(stop<T> a) {
        return a.value;
    }

} // namespace rules_helper

template<Rule auto... rs> struct rules {
    REPRESENTS(Rule)

    static constexpr auto the_chain = chain(rules_helper::make_fun(rs)...);
    constexpr auto operator()(CtStr auto t) const {
        return rules_helper::make_res(the_chain(rules_helper::make_arg(t)));
    }
};
template<> struct rules<> {
    REPRESENTS(Rule)
    constexpr auto operator()(CtStr auto t) const { return fail{}; }
};

// loop

namespace loop_helper {
    // implements endless loop on text >> rule >> rule >> ...

    constexpr auto make_arg(CtStr auto text) { return arg{text}; }
    constexpr auto make_fun(Rule auto rule) {
        // rule : CtStr -> CtStr | fail
        // fun  : arg<CtStr> -> arg<CtStr> | stop<CtStr>
        return [rule]<CtStr T>(arg<T> a) {
            auto r = rule(a.value);
            if constexpr (failed(r))  // not matched anymore
                return stop{a.value}; // stop with current value
            else
                return arg{r};        // success, continue
        };
    }
    template<CtStr T> constexpr fail make_res(arg<T> a) = delete;
    template<CtStr T> constexpr T make_res(stop<T> a) {
        return a.value;
    }

} // namespace loop_helper

template<Rule auto p> struct rule_loop {
    REPRESENTS(Rule)

    static constexpr auto the_loop = endless_loop{repeat<10>(loop_helper::make_fun(p))};

    constexpr auto operator()(CtStr auto t) const {
        return loop_helper::make_res(the_loop(loop_helper::make_arg(t)));
    }
};

#define STR(s) (str{s}) // s##_ss
#define CTSTR(s) (ct<STR(s)>{}) // s##_cts
#define RULE(s, r) (rule<STR(s), STR(r)>{})
#define RULES(...) rules<__VA_ARGS__>{}

// To hide a program from compiler output
#define NAMED_RULE(name, p) \
(struct name { \
    REPRESENTS(Rule) \
    constexpr auto operator()(CtStr auto t) const { return (p)(t); } \
}){}

