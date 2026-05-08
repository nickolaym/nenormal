#pragma once

#include "rule_concepts.h"

// named rule is a temporary class, not a template, which reduces compiler output
// (especially in cases of errors)
// (and maybe reduces lengths of linker symbols)

#define NAMED_RULE_TYPE(name, p) \
struct name { \
    REPRESENTS(Rule) \
    static constexpr auto impl = (p); \
    constexpr decltype(auto) operator()(RuleNotMatchedYetInputRef auto&& nmy) const { return impl(FWD(nmy)); } \
    constexpr RuleOutput auto operator()(RuleInputRef auto&& t) const { return impl(FWD(t)); } \
    constexpr auto operator()(RuleFixedInput auto& t) const { return impl(t); } \
}

#define NAMED_RULE(name, p) ( (NAMED_RULE_TYPE(name, p)) {} )
