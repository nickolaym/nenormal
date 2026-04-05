#include <iostream>
#include "static_string.h"

using namespace ss::literals;

template<ss::StringValueType auto s, ss::StringValueType auto r, bool h>
requires (h || s != r)
struct entrytype
{
    static constexpr auto search = s;
    static constexpr auto replace = r;
    static constexpr auto halt = h;

    static constexpr bool matches(ss::StringValueType auto t) {
        if constexpr (s == ""_ssv) {
            return true;  // empty search always succeeds.
        } else {
            return ss::find(t, s)() != t().size();
        }
    }

    constexpr auto operator()(ss::StringValueType auto t) const requires (matches(t))
    {
        return std::pair{ss::replace(t, s, r), h};
    }
};

template<auto s, auto r, bool b> constexpr auto entry = entrytype<s, r, b>{};
constexpr auto entry_stop = entry<""_ssv, ""_ssv, true>;

template<auto s, auto r, bool h> constexpr bool check_entry(entrytype<s, r, h> e) { return true; }
template<class E> concept Entry = requires (E e) { check_entry(e); };


template<auto... Fs> struct program;
template<> struct program<> {
    // void operator()(auto) const {}
    constexpr auto operator()(auto a) const {
        std::cout << a().data() << " == default halt" << std::endl;
        return entry_stop(a);
    }
};
template<auto F, auto... Fs> struct program<F, Fs...> : program<Fs...> {
    constexpr auto operator()(auto a) const /*requires requires { F(a); }*/ {
        std::cout << a().data() << " /" << F.search().data() << "/" << F.replace().data() << "/ " << (F.halt ? "halt" : "next") << std::endl;
        if constexpr(F.matches(a)) {
            std::cout << " == " << F(a).first().data() << std::endl;
            return F(a);
        } else {
            return program<Fs...>::operator()(a);
        }
    }
};


constexpr auto entry_world_mir = entry<"world"_ssv, "mir"_ssv, false>;
constexpr auto entry_l = entry<"l"_ssv, "L"_ssv, false>;

struct qq : program<
    entry_world_mir,
    entry_l
    // entry_stop
> {};

int main() {
    qq q;

    auto t1 = "hello world!!!"_ssv;
    auto t2 = q(t1).first;
    auto t3 = q(t2).first;
    auto t4 = q(t3).first;
    auto t5 = q(t4).first;
    auto t6 = q(t5).first;
}
