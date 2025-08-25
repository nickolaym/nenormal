#pragma once

#include <cstdlib>
#include <utility>

namespace ss {

    template<class... Fs> class compose;
    
    template<> class compose<> {
        void operator()(auto) const = delete;
    };
    
    template<class F> class compose<F> {
        F f;
    public:
        explicit constexpr compose(F f) : f(f) {}
        constexpr auto operator()(auto arg) const requires requires { f(arg); } { return f(arg); }
    };

    template<class F, class... Fs> class compose<F, Fs...> {
        F f;
        compose<Fs...> g;
    public:
        explicit constexpr compose(F f, Fs... fs) : f(f), g(fs...) {}
        constexpr auto operator()(auto arg) const requires requires { f(arg); } || requires { g(arg); } {
            if constexpr (requires { f(arg); }) {
                return f(arg);
            } else {
                return g(arg);
            }
        }
    };

    template<class... Fs> compose(Fs...) -> compose<Fs...>;

} // namespace ss
