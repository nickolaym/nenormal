#pragma once

#include "concepts.h"
#include "str.h"

// augmentation is a function(input, output) returning new augmentation

struct empty {
    constexpr auto operator()(CtStr auto i, CtStr auto s, CtStr auto r, CtStr auto o) const { return *this; }
};

template<class F>
struct side_effect {
    F f;
    constexpr auto operator()(CtStr auto i, CtStr auto s, CtStr auto r, CtStr auto o) const {
        f(i, s, r, o);
        return side_effect{std::move(f)};
    }
};

template<class F, class A>
struct cumulative_effect {
    F f;
    A a;
    constexpr auto operator()(CtStr auto i, CtStr auto s, CtStr auto r, CtStr auto o) const {
        auto b = f(a, i, s, r, o);
        return cumulative_effect<F, decltype(b)>{std::move(f), std::move(b)};
    }
};

///

template<CtStr T, class A>
struct augmented_text {
    REPRESENTS(Augmented)
    T text;
    A aux;

    constexpr auto operator()(CtStr auto s, CtStr auto r, CtStr auto new_text) const {
        auto new_aux = aux(text, s, r, new_text);
        auto res = augmented_text<decltype(new_text), decltype(new_aux)>{new_text, new_aux};
        return res;
    }
};
CONCEPT(Augmented)

///

constexpr CtStr auto extract_text(CtStr auto i) { return i; }
constexpr CtStr auto extract_text(Augmented auto const& i) { return i.text; }

constexpr CtStr auto update_text(CtStr auto i, CtStr auto s, CtStr auto r, CtStr auto o) {
    return o;
}
constexpr Augmented auto update_text(Augmented auto i, CtStr auto s, CtStr auto r, CtStr auto o) {
    return i(s, r, o);
}
