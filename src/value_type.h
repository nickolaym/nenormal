#pragma once

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

    // comparison

    constexpr bool operator == (const ValueType auto& x, const ValueType auto& y)
    requires requires { x() == y(); } {
        // comparison of subsequent types, like
        // return std::is_same_v<decltype(x), decltype(y)>;
        // is dangerous, because it allows mixing,
        // like value<cstring("foo")> == value<123>
        // (with answer false, certainly)
        return x() == y();
    }

}  // namespace ss
