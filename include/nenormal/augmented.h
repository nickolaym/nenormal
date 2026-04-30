#pragma once

#include "concepts.h"
#include "str.h"

// augmentation is a self-updating function of a single rule "p"

CONCEPT(Augmentation)

struct empty {
    REPRESENTS(Augmentation)
    constexpr auto operator()(CtStr auto i, auto p, CtStr auto o) const {
        return empty{};
    }
    constexpr bool operator == (empty const&) const = default;
};

template<class F>
struct side_effect {
    REPRESENTS(Augmentation)
    F f;
    constexpr auto operator()(CtStr auto i, auto p, CtStr auto o) const {
        f(i, p, o);
        return side_effect{std::move(f)};
    }

    constexpr bool operator == (side_effect const&) const { return true; }
};

template<class F, class A>
struct cumulative_effect {
    REPRESENTS(Augmentation)
    F f;
    A a;
    constexpr auto operator()(CtStr auto i, auto p, CtStr auto o) const {
        auto b = f(a, i, p, o);
        return cumulative_effect<F, decltype(b)>{std::move(f), std::move(b)};
    }

    constexpr bool operator == (cumulative_effect const& v) const { return a == v.a; }

    template<class A1>
    constexpr bool operator == (cumulative_effect<F,A1> const& v) const { return a == v.a; }
};

/////////////////////////////////////
// augmented_text provides two modes:
// - update augmentation (invoke side effect function)
// - simply rebind without application

template<CtStr T, Augmentation A>
struct augmented_text {
    REPRESENTS(Augmented)
    T text;
    A aux;

    constexpr bool operator == (augmented_text const&) const = default;
    template<CtStr T1, Augmentation A1>
    constexpr bool operator == (augmented_text<T1,A1> const& v) const {
        return text == v.text && aux == v.aux;
    }

    constexpr auto update(auto p, CtStr auto new_text) const {
        auto new_aux = aux(text, p, new_text);
        auto res = augmented_text<decltype(new_text), decltype(new_aux)>{new_text, new_aux};
        return res;
    }
    constexpr auto rebind(CtStr auto new_text) const {
        return augmented_text<decltype(new_text), A>{new_text, aux};
    }
};
CONCEPT(Augmented)

// extract and restore text from an augmented type

constexpr CtStr auto extract_text(CtStr auto i) { return i; }
constexpr CtStr auto extract_text(Augmented auto const& i) { return i.text; }

constexpr CtStr auto update_text(CtStr auto i, auto p, CtStr auto o) {
    return o;
}
constexpr Augmented auto update_text(Augmented auto i, auto p, CtStr auto o) {
    return i.update(p, o);
}

constexpr CtStr auto rebind_text(CtStr auto i, CtStr auto o) {
    return o;
}
constexpr Augmented auto rebind_text(Augmented auto i, CtStr auto o) {
    return i.rebind(o);
}
