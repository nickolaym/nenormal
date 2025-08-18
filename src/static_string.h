#pragma once

#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <array>
#include <type_traits>

namespace ss {

    // true compile time constant
    template<auto V> struct valuetype {
        using type = decltype(V);
        static constexpr auto value = V;
        constexpr auto operator()() const { return value; }
    };

    template<auto V> constexpr auto value = valuetype<V>{};

    template<class T> constexpr bool is_valuetype_v = false;
    template<auto V> constexpr bool is_valuetype_v<valuetype<V>> = true;
    template<class T> concept ValueType = is_valuetype_v<T>;

    // c-string - ad-hoc array
    template<size_t N> struct cstring {
        char data_[N + 1] = {};
        constexpr static size_t size() { return N; }
        constexpr auto& data() { return data_; }
        constexpr const auto& data() const { return data_; }
        constexpr auto begin() { return data_; }
        constexpr auto begin() const { return data_; }
        constexpr auto end() { return data_ + N; }
        constexpr auto end() const { return data_ + N; }
        constexpr bool operator == (const cstring&) const = default;
        template<size_t M> constexpr bool operator == (const cstring<M>&) const { return false; }
    };

    template<class T> constexpr bool is_cstring_v = false;
    template<size_t N> constexpr bool is_cstring_v<cstring<N>> = true;
    template<class T> concept CString = is_cstring_v<T>;

    template<CString auto V> using string_valuetype = valuetype<V>;
    template<CString auto V> constexpr auto string_value = value<V>;
    template<class T> concept StringValueType = ValueType<T> && is_cstring_v<typename T::type>;

    // size_t as a compile time constant
    template<size_t N> using size_valuetype = valuetype<N>;
    template<size_t N> constexpr auto size_value = value<N>;
    template<class T> concept SizeValueType = ValueType<T> && std::same_as<size_t, typename T::type>;

    ///////////////////
    // construct string

    template<size_t N>
    constexpr CString auto make_cstring(const char(&s)[N]) {
        cstring<N-1> v;
        std::copy_n(s, N-1, v.begin());
        return v;
    }

    constexpr bool operator == (const ValueType auto& x, const ValueType auto& y) {
        return std::is_same_v<decltype(x), decltype(y)>;
    }

    constexpr CString auto operator + (const CString auto& x, const CString auto& y) {
        constexpr auto nx = x.size();
        constexpr auto ny = y.size();
        cstring<nx + ny> z;
        std::copy_n(x.begin(), nx, z.begin());
        std::copy_n(y.begin(), ny, z.begin() + nx);
        return z;
    }

    constexpr StringValueType auto operator + (StringValueType auto x, StringValueType auto y) {
        return value<x() + y()>;
    }

    constexpr size_t find(const CString auto& x, const CString auto& y) {
        if (x.size() < y.size()) return x.size();
        return std::distance(x.begin(), std::search(x.begin(), x.end(), y.begin(), y.end()));
    }

    constexpr SizeValueType auto find(StringValueType auto x, StringValueType auto y) {
        return ss::value<find(x(), y())>;
    }

    // substr returns a string of new unknown size
    // that's why we must pass integral arg as value_type
    
    constexpr CString auto substr(const CString auto& x, SizeValueType auto vp, SizeValueType auto vn) {
        constexpr size_t p = vp();
        constexpr size_t n = vn();
        static_assert(x.size() >= p + n);  // or crop
        
        ss::cstring<n> y;
        std::copy_n(x.begin() + p, n, y.begin());
        return y;
    }
    constexpr CString auto substr(const CString auto& x, SizeValueType auto vp) {
        return substr(x, vp, ss::size_value<x.size() - vp()>);
    }
    constexpr StringValueType auto substr(StringValueType auto vx, SizeValueType auto vp, SizeValueType auto vn) {
        return value<substr(vx(), vp, vn)>;
    }
    constexpr StringValueType auto substr(StringValueType auto vx, SizeValueType auto vp) {
        return value<substr(vx(), vp)>;
    }

    // replace returns a string of unknown size
    // depending on input strings
    // that's why all of them are of value_type

    constexpr auto replace(StringValueType auto vx, StringValueType auto vy, StringValueType auto vz) {
        constexpr auto vp = find(vx, vy);
        if constexpr (vp() == vx().size()) {
            return vx;
        } else {
            return (
                substr(vx, size_value<0>, vp) +
                vz +
                substr(vx, size_value<vp + vy().size()>)
            );
        }
    }

}  // namespace ss
