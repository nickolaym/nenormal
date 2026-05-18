#pragma once

#include "str.h"

namespace nn {

template<size_t n> using ct_size = ct<n>;
template<size_t n> constexpr auto ct_size_v = ct_size<n>{};
template<class T> concept CtSize = CtOfType<T, size_t>;

template<char c> using ct_char = ct<c>;
template<char c> constexpr auto ct_char_v = ct_char<c>{};
template<class T> concept CtChar = CtOfType<T, char>;

constexpr Str auto chars(CtSize auto n, char c) {
    constexpr size_t size = n.value;
    str<size> res;
    auto it = res.begin();
    it = std::fill_n(it, size, c);
    *it = 0;
    return res;
}

constexpr CtStr auto ct_chars(CtSize auto n, CtChar auto c) {
    return ct<chars(n, c.value)>{};
}

constexpr Str auto concat_str(Str auto const&... ss) {
    if constexpr (sizeof...(ss) == 0) {
        return str{""};
    } else if constexpr (sizeof...(ss) == 1) {
        return (ss , ...); // pack of single item unrolls to that item
    } else {
        constexpr size_t total_size = (ss.size() + ... + 0);
        str<total_size> res;
        auto it = res.begin();
        ((it = std::copy(ss.begin(), ss.end(), it)) , ...);
        *it = 0;
        return res;
    }
}

constexpr CtStr auto concat_ctstr(CtStr auto... ss) {
    return ct<concat_str(ss.value ...)>{};
}

} // namespace nn
