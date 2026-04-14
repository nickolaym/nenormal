#pragma once

// if an expression cannot be resolved (immediate error)
// it's impossible to put it into static_assert directly.
// incapsulation into a template lambda moves it to the instantiation time.
#define WILL_NOT_RESOLVE(expr) [&](auto){ return !requires{ (expr); }; }(0)
