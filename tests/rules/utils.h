#pragma once

#include "nenormal/nenormal.h"
#include <utility>

constexpr auto call_inplace_ex(::nn::Rule auto p, ::nn::RuleFixedInput auto a) {
    ::nn::tristate_kind k = p.update(a);
    return ::nn::inplace_argument{a, k};
}

constexpr auto call_inplace(::nn::Rule auto p, ::nn::RuleFixedInput auto a) {
    p.update(a);
    return a;
}
