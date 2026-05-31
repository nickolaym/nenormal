#pragma once

// if an expression cannot be resolved (immediate error)
// it's impossible to put it into static_assert directly.
// incapsulation into a template lambda moves it to the instantiation time.
#define WILL_NOT_RESOLVE(expr) [&](auto) { return !requires { (expr); }; }(0)

#define STATIC_ASSERT_EQ(a, b) static_assert(a == b, #a " != " #b)

#define STATIC_ASSERT_EQ_TYPE(a, b) static_assert(std::is_same_v<a, b> , #a " != " #b)
