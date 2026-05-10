#pragma once

#include "concepts.h"

namespace nn {

// functional compostions with fold expressions

CONCEPT(Arg);
CONCEPT(Stop);
template<class T> concept ArgOrStop = Arg<T> || Stop<T>;

template<class T> struct arg;
template<class T> struct stop;

template<class T> struct arg {
    REPRESENTS(Arg);
    T value;
    using type = T;

    constexpr bool operator == (const arg&) const = default;
};
template<class T> struct stop {
    REPRESENTS(Stop);
    T value;
    using type = T;

    constexpr bool operator == (const stop&) const = default;
};

// simple composition ax >> fun where fun : arg<T> -> arg<U> | stop<U>

template<class T, class F> constexpr auto operator>>(arg<T> a, F f) { return f(a); }
template<class T, class F> constexpr auto operator>>(stop<T> s, F f) { return s; }

// primitive functions (named types for verbose debugging)

constexpr auto id = (struct id {
    template<class T> constexpr auto operator()(arg<T> a) const { return a; }
}) {};
constexpr auto interrupt = (struct interrupt {
    template<class T> constexpr auto operator()(arg<T> a) const { return stop{a.value}; }
}) {};

// chain of functions

constexpr auto chain(auto... fs) {
    return [=]<class T>(arg<T> a) {
        return (a >> ... >> fs);
    };
}

// recursive repetition of a function

template<size_t N> struct repeat_t;
template<size_t N> constexpr auto repeat = repeat_t<N>{};

template<size_t N> struct repeat_t {
    constexpr auto operator()(auto f) const {
        if constexpr (N == 0) return id;
        else if constexpr (N == 1) return f;
        else if constexpr (N == 2) return chain(f, f);
        else if constexpr (N == 3) return chain(f, f, f);
        else if constexpr (N == 4) return chain(f, f, f, f);
        else if constexpr (N == 5) return chain(f, f, f, f, f);
        else if constexpr (N == 6) return chain(f, f, f, f, f, f);
        else if constexpr (N == 7) return chain(f, f, f, f, f, f, f);
        else if constexpr (N == 8) return chain(f, f, f, f, f, f, f, f);
        else if constexpr (N == 9) return chain(f, f, f, f, f, f, f, f, f);
        else if constexpr (N == 10) return chain(f, f, f, f, f, f, f, f, f, f);
        else if constexpr (N % 10 != 0)
            return chain(
                repeat<N % 10>(f),
                [f](auto a) { return a >> repeat<N - N % 10>(f); }
            );
            // return chain(repeat<N - N % 10>(f), repeat<N % 10>(f));
        else
            return repeat<N / 10>(repeat<10>(f));
    }
};

// simple endless loop with natural limitation of recursion
// possible usage: endless_loop{repeat<100>(f)}

template<class F>
struct endless_loop {
    F f;
    constexpr auto operator()(auto a) const { return a >> f >> *this; }
};

// extending endless loop with nearly-fibonaccy grow factor
// possible usage: extending_endless_loop{f}

template<class F>
struct extending_endless_loop {
    F f;

    template<size_t N> struct impl {
        F f;
        constexpr auto operator()(auto a) const {
            constexpr size_t Next = (N == 1) ? 20 : (N * 1.5);
            return a >> repeat<N>(f) >> impl<Next>{f};
        }
    };

    constexpr auto operator()(auto a) const {
        return a >> impl<1>{f};
    }
};

} // namespace nn
