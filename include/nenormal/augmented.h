#pragma once

#include "concepts.h"
#include "str.h"
#include "utility.h"

namespace nn {

// augmentation is a self-updating function of a single rule "p"

CONCEPT(Augmentation)

struct empty {
    REPRESENTS(Augmentation)
    constexpr auto operator()(CtStr auto i, auto p, CtStr auto o) const {
        return empty{};
    }
    constexpr bool operator == (empty const&) const = default;
};

template<class A>
struct passed {
    REPRESENTS(Augmentation)
    A a;

    constexpr auto operator()(CtStr auto i, auto p, CtStr auto o) const& {
        return *this;
    }
    constexpr auto operator()(CtStr auto i, auto p, CtStr auto o) && {
        return FWD(*this);
    }

    constexpr bool operator == (passed const&) const = default;

    constexpr bool operator == (Augmentation auto const& other) const
        requires requires { a == other.a; }
    { return a == other.a; }

    friend constexpr bool operator == (Augmentation auto const& lhs, passed const& rhs)
        requires requires { lhs.a == rhs.a; }
    { return lhs.a == rhs.a; }
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

template<class A, class F>
struct cumulative_effect {
    REPRESENTS(Augmentation)
    // this order of members is intentional.
    // it lets eliminate extra move constructor with aggregate initialization
    // of next value: cumulative_effect{f(move(a)), move(f)}
    A a;
    F f;

    constexpr auto operator()(CtStr auto i, auto p, CtStr auto o) && {
        using new_acc_type = decltype(f(std::move(a), i, p, o));
        // CTAD does not work here because cumulative_effect{} corresponds to current
        using new_effect = cumulative_effect<new_acc_type, F>;
        // no race condition because the order of initialization is always natural
        // (the order of declaration)
        return new_effect{
            .a = f(std::move(a), i, p, o),
            .f = std::move(f)
        };
    }
    constexpr auto operator()(CtStr auto i, auto p, CtStr auto o) const& {
        using new_acc_type = decltype(f(a, i, p, o));
        // CTAD does not work here because cumulative_effect{} corresponds to current
        return cumulative_effect<new_acc_type, F>{f(a, i, p, o), f};
    }

    constexpr bool operator == (cumulative_effect const& v) const
        requires std::equality_comparable<A>
    { return a == v.a; }

    template<class A1>
    constexpr bool operator == (cumulative_effect<A1, F> const& v) const
        requires std::equality_comparable_with<A, A1>
    { return a == v.a; }
};

////////////////////////////////////////
// debug purposes: technical side effect

CONCEPT(DebugAugmentation)

template<Augmentation A, class D>
struct debug_augmentation {
    REPRESENTS(Augmentation)
    REPRESENTS(DebugAugmentation)
    A basic_aux;
    D debug_callback; // void(std::string_view)

    constexpr auto operator()(CtStr auto i, auto p, CtStr auto o) && {
        using new_aux_type = decltype(::std::move(basic_aux)(i, p, o));
        return debug_augmentation<new_aux_type, D>{
            ::std::move(basic_aux)(i, p, o),
            ::std::move(debug_callback)
        };
    }
    constexpr auto operator()(CtStr auto i, auto p, CtStr auto o) const& {
        using new_aux_type = decltype(basic_aux(i, p, o));
        return debug_augmentation<new_aux_type, D>{
            basic_aux(i, p, o),
            debug_callback
        };
    }
};

/////////////////////////////////////
// augmented_text provides two modes:
// - update augmentation (invoke side effect function)
// - simply rebind without application

CONCEPT(Augmented)

template<CtStr T, Augmentation A>
struct augmented_text {
    REPRESENTS(Augmented)
    T text;
    A aux;

    static constexpr CtStr auto the_text = T{};

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

    constexpr auto update(auto&& p, CtStr auto new_text) &&
    // requires std::move_constructible<A>
    {
        using new_text_type = decltype(new_text);
        using new_aux_type = decltype(std::move(aux)(text, p, new_text));
        // CTAD does not work here because augmented_text{} corresponds to current
        return augmented_text<new_text_type, new_aux_type>{
            new_text,
            std::move(aux)(text, p, new_text),
        };
    }
    constexpr auto update(auto p, CtStr auto new_text) const&
    // requires std::copy_constructible<A>
    {
        using new_text_type = decltype(new_text);
        using new_aux_type = decltype(aux(text, p, new_text));
        // CTAD does not work here because augmented_text{} corresponds to current
        return augmented_text<new_text_type, new_aux_type>{
            new_text,
            aux(text, p, new_text),
        };
    }

    constexpr auto rebind(CtStr auto new_text) &&
    requires std::move_constructible<A>
    {
        return augmented_text<decltype(new_text), A>{new_text, std::move(aux)};
    }
    constexpr auto rebind(CtStr auto new_text) const&
    requires std::copy_constructible<A>
    {
        return augmented_text<decltype(new_text), A>{new_text, aux};
    }
};

// extract and restore text from an augmented type

constexpr CtStr auto extract_text(CtStr auto i) { return i; }
constexpr CtStr auto extract_text(Augmented auto const& i) { return i.the_text; }

constexpr CtStr auto update_text(CtStr auto i, auto p, CtStr auto o) {
    return o;
}
constexpr Augmented auto update_text(Augmented auto&& i, auto p, CtStr auto o) {
    return std::move(i).update(p, o);
}

constexpr CtStr auto rebind_text(CtStr auto i, CtStr auto o) {
    return o;
}
constexpr Augmented auto rebind_text(Augmented auto&& i, CtStr auto o) {
    return FWD(i).rebind(o);
}

// debug

constexpr void debug_call(CtStr auto i, auto&&... args) {}
constexpr void debug_call(Augmented auto&& i, auto&&... args) {
    if constexpr(
        DebugAugmentation<decltype(i.aux)> &&
        requires { i.aux.debug_callback(FWD(args)...); }
    ) {
        i.aux.debug_callback(FWD(args)...);
    }
}

constexpr auto dummy_debug_callback = [](auto&&...) {};
constexpr auto get_debug_callback(const CtStr auto& i) {
    return dummy_debug_callback;
}
constexpr auto get_debug_callback(const Augmented auto& i) {
    if constexpr (DebugAugmentation<decltype(i.aux)>) {
        return [cb = i.aux.debug_callback](auto&&... args) {
            if constexpr (requires { cb(FWD(args)...); }) {
                cb(FWD(args)...);
            }
        };
    } else {
        return dummy_debug_callback;
    }
}

} // namespace nn
