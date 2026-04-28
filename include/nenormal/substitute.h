#pragma once

#include "ct.h"
#include "str.h"
#include "maybe.h"
#include <algorithm>

template<class T> concept JustCtStr = Just<T> && CtStr<typename T::type>;
template<class T> concept MaybeCtStr = Nothing<T> || JustCtStr<T>;

// search, replace, text -> maybe new text
constexpr MaybeCtStr auto try_substitute(CtStr auto cts, CtStr auto ctr, CtStr auto ctt) {
    constexpr Str auto const& src = ctt.value;
    constexpr Str auto const& s = cts.value;
    constexpr Str auto const& r = ctr.value;

    if constexpr (src.size() < s.size()) {
        return nothing{};
    } else if constexpr (src == s) {
        return just{ctr}; // total replacement
    } else if constexpr (s.empty() && r.empty()) {
        return just{ctt}; // nothing to search and replace
    } else {
        constexpr auto fbegin = std::search(src.begin(), src.end(), s.begin(), s.end());
        if constexpr (fbegin == src.end()) {
            return nothing{};
        } else {
            constexpr Str auto dst = // constexpr constant
                [&]{
                    auto fend = fbegin + s.size();
                    constexpr auto len = src.size() - s.size() + r.size();
                    str<len + 1> dst; // not constant yet in this block
                    auto it = dst.begin();
                    it = std::copy(src.begin(), fbegin, it);
                    it = std::copy(r.begin(), r.end(), it);
                    it = std::copy(fend, src.end(), it);
                    *it = 0;
                    return dst;
                }();
            CtStr auto ctd = ct<dst>{};
            return just{ctd};
        }
    }
}
