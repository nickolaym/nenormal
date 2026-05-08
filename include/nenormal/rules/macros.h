#pragma once

// functional form of string literals
#define STR(s) (str{s}) // s##_ss
#define CTSTR(s) (ct<STR(s)>{}) // s##_cts

// single rule
#define RULE(s, r) (rule_v<STR(s), STR(r), regular_state>)
#define FINAL_RULE(s, r) (rule_v<STR(s), STR(r), final_state>)

// rule series
#define RULES(...) (rules_v<__VA_ARGS__>)

// augmentation wrappers
#define HIDDEN_RULE(p) (hidden_rule_v<(p)>)
#define FACADE_RULE(name, p) (facade_rule_v<STR(name), (p)>)

// rule loop
// (note that rule_loop_body is not a public building block)
#define RULE_LOOP(r) (rule_loop_v<(r)>)

// machine from arbitrary rule is for debug purposes (it plays single iteration only)
#define MACHINE_FROM_RULE(r) (machine_fun_v<(r)>)
// machine that runs a rule loop is typical use
#define MACHINE(r) MACHINE_FROM_RULE(RULE_LOOP(r))
