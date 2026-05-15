#pragma once

#include "rule_concepts.h"

// named rule is a temporary class, not a template, which reduces compiler output
// (especially in cases of errors)
// (and maybe reduces lengths of linker symbols)

#define NAMED_RULE_TYPE(name, p) \
struct name { \
    REPRESENTS(::nn::Rule) \
    static constexpr ::nn::Rule auto impl = (p); \
    constexpr ::nn::RuleOutput decltype(auto) \
        operator()(::nn::RuleNotMatchedYetInput auto&& nmy) const { return impl(FWD(nmy)); } \
    constexpr ::nn::RuleOutput auto \
        operator()(::nn::MachineData auto&& t) const { return impl(FWD(t)); } \
    constexpr auto operator()(::nn::RuleFixedInput auto& t) const { return impl(t); } \
}

#define NAMED_RULE(name, p) ( (NAMED_RULE_TYPE(name, p)) {} )
