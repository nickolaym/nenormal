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
    constexpr auto operator()(CtStr auto i, auto p, CtStr auto o) && {
        f(i, p, o);
        return side_effect{std::move(f)};
    }
    constexpr auto operator()(CtStr auto i, auto p, CtStr auto o) const& {
        f(i, p, o);
        return side_effect{f};
    }

    constexpr bool operator == (side_effect const&) const { return true; }
};

template<class F, class A>
struct cumulative_effect {
    REPRESENTS(Augmentation)
    F f;
    A a;

    constexpr auto operator()(CtStr auto i, auto p, CtStr auto o) && {
        // moving both f and f(...) is a race condition.
        // so we must evaluate the result first.
        auto new_acc = f(std::move(a), i, p, o);
        using new_acc_type = decltype(new_acc);
        // CTAD does not work here because cumulative_effect{} corresponds to current
        return cumulative_effect<F, new_acc_type>{std::move(f), std::move(new_acc)};
    }
    constexpr auto operator()(CtStr auto i, auto p, CtStr auto o) const& {
        using new_acc_type = decltype(f(a, i, p, o));
        // CTAD does not work here because cumulative_effect{} corresponds to current
        return cumulative_effect<F, new_acc_type>{f, f(a, i, p, o)};
    }

    constexpr bool operator == (cumulative_effect const& v) const
        requires std::equality_comparable<A>
    { return a == v.a; }

    template<class A1>
    constexpr bool operator == (cumulative_effect<F,A1> const& v) const
        requires std::equality_comparable_with<A, A1>
    { return a == v.a; }
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

    // cannot easily guarantee safety of references
    // so require that A is a value type
    static_assert(!std::is_reference_v<A>);

    constexpr bool operator == (augmented_text const&) const
        requires std::equality_comparable<A>
        = default;
    template<CtStr T1, Augmentation A1>
    constexpr bool operator == (augmented_text<T1,A1> const& v) const
        requires std::equality_comparable_with<A, A1>
    {
        return text == v.text && aux == v.aux;
    }

    constexpr auto update(auto&& p, CtStr auto new_text) && {
        using new_text_type = decltype(new_text);
        using new_aux_type = decltype(std::move(aux)(text, p, new_text));
        // CTAD does not work here because augmented_text{} corresponds to current
        return augmented_text<new_text_type, new_aux_type>{
            new_text,
            std::move(aux)(text, p, new_text),
        };
    }
    constexpr auto update(auto p, CtStr auto new_text) const& {
        using new_text_type = decltype(new_text);
        using new_aux_type = decltype(aux(text, p, new_text));
        // CTAD does not work here because augmented_text{} corresponds to current
        return augmented_text<new_text_type, new_aux_type>{
            new_text,
            aux(text, p, new_text),
        };
    }

    constexpr auto rebind(CtStr auto new_text) && {
        return augmented_text<decltype(new_text), A>{new_text, std::move(aux)};
    }
    constexpr auto rebind(CtStr auto new_text) const& {
        return augmented_text<decltype(new_text), A>{new_text, aux};
    }
};
CONCEPT(Augmented)
template<class R> concept AugmentedRef = Augmented<std::remove_cvref_t<R>>;

// extract and restore text from an augmented type

constexpr CtStr auto extract_text(CtStr auto i) { return i; }
constexpr CtStr auto extract_text(AugmentedRef auto&& i) { return i.text; }

constexpr CtStr auto update_text(CtStr auto i, auto p, CtStr auto o) {
    return o;
}
constexpr Augmented auto update_text(AugmentedRef auto&& i, auto p, CtStr auto o) {
    return i.update(p, o);
}

constexpr CtStr auto rebind_text(CtStr auto i, CtStr auto o) {
    return o;
}
constexpr Augmented auto rebind_text(AugmentedRef auto&& i, CtStr auto o) {
    return i.rebind(o);
}
