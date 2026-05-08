#pragma once

#include "rule_concepts.h"

// machine is a function string -> string,
// without technical details about regular / final state.
// it just puts data into Tristate monad, runs it there, and takes result back.

// typical use is a wrapping rules with rule_loop and machine_fun.

template<Rule auto p> struct machine_fun {
    // rvalue-ref input to optimize a bit
    constexpr RuleInput auto operator()(RuleInputRef auto&& t) const {
        return (not_matched_yet{FWD(t)} >> p).value;
    }
    // rvalue input because inside it works as a variable
    constexpr RuleFixedInput auto operator()(RuleFixedInput auto t) const {
        p(t);
        return std::move(t);
    }
};

template<Rule auto m> constexpr machine_fun<m> machine_fun_v{};
