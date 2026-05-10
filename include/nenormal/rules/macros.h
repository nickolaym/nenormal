#pragma once

// single rule
#define RULE(s, r) (::nn::rule_v<STR(s), STR(r), ::nn::regular_state>)
#define FINAL_RULE(s, r) (::nn::rule_v<STR(s), STR(r), ::nn::final_state>)

// rule series
#define RULES(...) (::nn::rules_v<__VA_ARGS__>)

// augmentation wrappers
#define HIDDEN_RULE(p) (::nn::hidden_rule_v<(p)>)
#define FACADE_RULE(name, p) (::nn::facade_rule_v<STR(name), (p)>)

// rule loop
// (note that rule_loop_body is not a public building block)
#define RULE_LOOP(r) (::nn::rule_loop_v<(r)>)

// machine from arbitrary rule is for debug purposes (it plays single iteration only)
#define MACHINE_FROM_RULE(r) (::nn::machine_fun_v<(r)>)
// machine that runs a rule loop is typical use
#define MACHINE(r) MACHINE_FROM_RULE(RULE_LOOP(r))
