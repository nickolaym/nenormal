#pragma once
#include "utility.h"

namespace nn {

// scope exit without boost

struct scope_exit_launcher {
    template<class F> struct impl {
        F f;
        constexpr impl(F&& f) noexcept : f(FWD(f)) {}
        constexpr impl(impl&&) noexcept = default; // moveable, noncopyable
        constexpr ~impl() { f(); }
    };

    constexpr auto operator << (auto&& f) const noexcept {
        return impl{FWD(f)};
    }
};

#define SCOPE_EXIT() auto&& [_] = scope_exit_launcher{} << [&]() /* { body goes here }; */

} // namespace nn
