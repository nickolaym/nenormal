#pragma once

#include "concepts.h"

// functional compostions with fold expressions

CONCEPT(Arg);
CONCEPT(Stop);
template<class T> concept ArgOrStop = Arg<T> || Stop<T>;

template<class T> struct arg;
template<class T> struct stop;

template<class T> struct arg {
    REPRESENTS(Arg);
    T value;

    constexpr bool operator == (const arg&) const = default;
};
template<class T> struct stop {
    REPRESENTS(Stop);
    T value;

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

// another strategy: nearly endless loop, extended on demand
template<size_t Limit> struct endless_t {
    constexpr auto operator()(auto f) const {
        constexpr size_t Factor = 10;
        if constexpr (Limit <= Factor)
            return repeat<Limit>(f);
        else {
            constexpr size_t N = (Limit - 1) % Factor + 1; // 1..10
            constexpr size_t R = (Limit - N) / Factor;
            static_assert(1 <= N && N <= Factor);
            static_assert(1 <= R);
            static_assert(N + R * Factor == Limit);
            return chain(
                repeat<N>(f),
                [f](auto a) { return a >> endless_t<R>{}(repeat<Factor>(f)); }
            );
        }
    }
};
template<size_t Limit> constexpr auto endless = endless_t<Limit>{};