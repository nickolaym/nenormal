#pragma once

#include "../concepts.h"
#include <string>

namespace nn {

CONCEPT(InplaceAugmentation)

struct inplace_empty {
    REPRESENTS(InplaceAugmentation);
    void operator()(auto p, std::string const& t) const {}
};

template<class F>
struct inplace_side_effect {
    REPRESENTS(InplaceAugmentation);
    F f;
    void operator()(auto p, std::string const& t) const { f(p, t); }
};

template<class F, class A>
struct inplace_cumulative_effect {
    REPRESENTS(InplaceAugmentation);
    F f; // A f(A&& a, auto p, std::string const& t)
    A a;
    void operator()(auto p, std::string const& t) { a = f(a, p, t); }
};

template<class F, class A>
struct inplace_modification_effect {
    REPRESENTS(InplaceAugmentation);
    F f; // void f(A& a, auto p, std::string const& t)
    A a;
    void operator()(auto p, std::string const& t) { f(a, p, t); }
};

CONCEPT(InplaceAugmented);

template<InplaceAugmentation A>
struct inplace_augmented_text {
    REPRESENTS(InplaceAugmented);
    std::string text;
    A aux;
};

constexpr std::string& inplace_extract_text(std::string& t) { return t; }
constexpr std::string& inplace_extract_text(InplaceAugmented auto& t) { return t.text; }

constexpr void inplace_update_text(std::string& t, auto p) {}
constexpr void inplace_update_text(InplaceAugmented auto& t, auto p) { t.aux(p, t.text); }

} // namespace nn
