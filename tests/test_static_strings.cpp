#include "../src/static_string.h"

using namespace ss::literals;

#include <gtest/gtest.h>

TEST(types, static_asserts) {
    using cs4_t = ss::cstring<4>;
    static_assert(ss::CString<cs4_t>);
    constexpr auto cs4 = cs4_t{"abcd"};
    static_assert(cs4.size() == 4);
    static_assert(cs4.data()[4] == 0);
    using s4_t = ss::string_valuetype<cs4>;
    static_assert(ss::StringValueType<s4_t>);
    static_assert(s4_t::value == cs4);

    constexpr auto n123 = 123_sz;
    static_assert(std::is_same_v<decltype(+n123), size_t>);
    using n123_t = ss::size_valuetype<n123>;
    static_assert(ss::SizeValueType<n123_t>);
    static_assert(n123_t::value == n123);

    constexpr auto n123v = ss::size_value<n123>;
    static_assert(n123v() == n123);
}

TEST(comparable, right_and_wrong) {
    constexpr auto x1 = "alfa"_ssv;
    constexpr auto x2 = "bravo"_ssv;
    constexpr auto y1 = ss::value<123_sz>;
    constexpr auto y2 = ss::value<456_sz>;
    constexpr auto z1 = ss::value<123>;  // int

    // cannot test requires out of template,
    // because it would lead to compile error;
    // that's why we indirect it with a template lambda
    auto can_compare = [](auto p, auto q) { return requires{p == q;}; };
    
    static_assert(can_compare(x1, x2));
    static_assert(x1 != x2);
    
    static_assert(can_compare(y1, y2));
    static_assert(can_compare(y1, z1));
    static_assert(y1 != y2);
    static_assert(y1 == z1);

    static_assert(!can_compare(x1, y1));
}

///

TEST(make, empty_string) {
    constexpr auto s = ""_ss;
    static_assert(s.size() == 0);
    EXPECT_STREQ(s.data(), "");
}

TEST(make, some_string) {
    constexpr auto s = "alfa"_ss;
    static_assert(s.size() == 4);
    EXPECT_STREQ(s.data(), "alfa");
}

TEST(make_value, empty_string) {
    constexpr auto s = ""_ssv;
    static_assert(s().size() == 0);
    EXPECT_STREQ(s().data(), "");
}

TEST(make_value, some_string) {
    constexpr auto s = "alfa"_ssv;
    static_assert(s().size() == 4);
    EXPECT_STREQ(s().data(), "alfa");
}

///

TEST(compare, same) {
    constexpr auto x = "alfa"_ssv;
    constexpr auto y = "alfa"_ssv;
    static_assert(x == y);
    static_assert(x() == y());
}

TEST(compare, different_body) {
    constexpr auto x = "alfa"_ssv;
    constexpr auto y = "beta"_ssv;
    static_assert(x != y);
    static_assert(x() != y());
}

TEST(compare, different_size) {
    constexpr auto x = "alfa"_ssv;
    constexpr auto y = "bravo"_ssv;
    static_assert(x != y);
    static_assert(x() != y());
}

///

TEST(concat, empty_empty) {
    constexpr auto cx = ""_ss;
    constexpr auto vx = ss::value<cx>;

    constexpr auto cy = ""_ss;
    constexpr auto vy = ss::value<cy>;

    constexpr auto cz = cx + cy;
    static_assert(cz == ""_ss);

    constexpr auto vz = vx + vy;
    static_assert(vz == ss::value<cz>);
}

TEST(concat, empty_busy) {
    constexpr auto cx = ""_ss;
    constexpr auto vx = ss::value<cx>;

    constexpr auto cy = "beta"_ss;
    constexpr auto vy = ss::value<cy>;

    constexpr auto cz = cx + cy;
    static_assert(cz == "beta"_ss);

    constexpr auto vz = vx + vy;
    static_assert(vz == ss::value<cz>);
}

TEST(concat, busy_busy) {
    constexpr auto cx = "alfa"_ss;
    constexpr auto vx = ss::value<cx>;

    constexpr auto cy = "beta"_ss;
    constexpr auto vy = ss::value<cy>;

    constexpr auto cz = cx + cy;
    static_assert(cz == "alfabeta"_ss);

    constexpr auto vz = vx + vy;
    static_assert(vz == ss::value<cz>);
}

TEST(concat, three) {
    constexpr auto cx = "alfa"_ss;
    constexpr auto cy = "beta"_ss;
    constexpr auto cz = "gamma"_ss;
    
    constexpr auto cxyz = cx + cy + cz;
    static_assert(cxyz == "alfabetagamma"_ss);

    constexpr auto vxyz = ss::value<cx> + ss::value<cy> + ss::value<cz>;
    static_assert(vxyz == ss::value<cxyz>);
}

///

TEST(find, empty_empty) {
    constexpr auto cx = ""_ss;
    constexpr auto cy = ""_ss;

    constexpr auto p = ss::find(cx, cy);
    static_assert(p == 0);
}

TEST(find, same) {
    constexpr auto cx = "alfa"_ss;
    constexpr auto cy = "alfa"_ss;

    constexpr auto p = ss::find(cx, cy);
    static_assert(p == 0);

    constexpr auto vp = ss::find(ss::value<cx>, ss::value<cy>);
    static_assert(vp == ss::value<p>);
}

TEST(find, smaller) {
    constexpr auto cx = "alfa"_ss;
    constexpr auto cy = "alfabeta"_ss;

    constexpr auto p = ss::find(cx, cy);
    static_assert(p == cx.size());

    constexpr auto vp = ss::find(ss::value<cx>, ss::value<cy>);
    static_assert(vp == ss::value<p>);
}

TEST(find, inside) {
    constexpr auto cx1 = "alfa"_ss;
    constexpr auto cy = "beta"_ss;
    constexpr auto cx3 = "gamma"_ss;
    constexpr auto cx = cx1 + cy + cx3;

    constexpr auto p = ss::find(cx, cy);
    static_assert(p == cx1.size());

    constexpr auto vp = ss::find(ss::value<cx>, ss::value<cy>);
    static_assert(vp == ss::value<p>);
}

TEST(find, inside_many) {
    constexpr auto cx1 = "alfa"_ss;
    constexpr auto cy = "beta"_ss;
    constexpr auto cx3 = "gamma"_ss;
    constexpr auto cx = cx1 + cy + cx3 + cy + cx1;

    constexpr auto p = ss::find(cx, cy);
    static_assert(p == cx1.size());

    constexpr auto vp = ss::find(ss::value<cx>, ss::value<cy>);
    static_assert(vp == ss::value<p>);
}

///

TEST(substr, empty) {
    constexpr auto cx = ""_ss;
    constexpr auto vp = ss::size_value<0>;
    constexpr auto vn = ss::size_value<0>;

    constexpr auto cy = ss::substr(cx, vp, vn);
    static_assert(cy == ""_ss);
    
    constexpr auto vy = ss::substr(ss::value<cx>, vp, vn);
    static_assert(vy == ss::value<cy>);

    constexpr auto cz = ss::substr(cx, vp);
    static_assert(cz == ""_ss);

    constexpr auto vz = ss::substr(ss::value<cx>, vp);
    static_assert(vz == ss::value<cz>);
}

TEST(substr, whole) {
    constexpr auto cx = "alfa"_ss;
    constexpr auto vp = ss::size_value<0>;
    constexpr auto vn = ss::size_value<cx.size()>;

    constexpr auto cy = ss::substr(cx, vp, vn);
    static_assert(cy == cx);
    
    constexpr auto vy = ss::substr(ss::value<cx>, vp, vn);
    static_assert(vy == ss::value<cy>);

    constexpr auto cz = ss::substr(cx, vp);
    static_assert(cz == cx);

    constexpr auto vz = ss::substr(ss::value<cx>, vp);
    static_assert(vz == ss::value<cz>);
}

TEST(substr, part) {
    constexpr auto cx1 = "alfa"_ss;
    constexpr auto cx2 = "beta"_ss;
    constexpr auto cx3 = "gamma"_ss;
    constexpr auto cx = cx1 + cx2 + cx3;
    constexpr auto vp = ss::size_value<cx1.size()>;
    constexpr auto vn = ss::size_value<cx2.size()>;

    constexpr auto cy = ss::substr(cx, vp, vn);
    static_assert(cy == cx2);
    
    constexpr auto vy = ss::substr(ss::value<cx>, vp, vn);
    static_assert(vy == ss::value<cy>);

    constexpr auto cz = ss::substr(cx, vp);
    static_assert(cz == cx2 + cx3);

    constexpr auto vz = ss::substr(ss::value<cx>, vp);
    static_assert(vz == ss::value<cz>);
}

///

TEST(replace, empty_all) {
    constexpr auto vx = ""_ssv;
    constexpr auto vy = ""_ssv;
    constexpr auto vz = ""_ssv;

    constexpr auto ve = ""_ssv;  // expected

    constexpr auto vt = ss::replace(vx, vy, vz);
    static_assert(vt == ve);
}

TEST(replace, empty_text) {
    constexpr auto vx = ""_ssv;
    constexpr auto vy = "alfa"_ssv;
    constexpr auto vz = "bravo"_ssv;

    constexpr auto ve = ""_ssv;  // expected

    constexpr auto vt = ss::replace(vx, vy, vz);
    static_assert(vt == ve);
}

TEST(replace, mismatched) {
    constexpr auto vx = "hello world"_ssv;
    constexpr auto vy = "alfa"_ssv;
    constexpr auto vz = "bravo"_ssv;

    constexpr auto ve = vx;  // expected

    constexpr auto vt = ss::replace(vx, vy, vz);
    static_assert(vt == ve);
}

TEST(replace, full_match) {
    constexpr auto vx = "hello world"_ssv;
    constexpr auto vy = vx;
    constexpr auto vz = "privet mir"_ssv;

    constexpr auto ve = vz;  // expected

    constexpr auto vt = ss::replace(vx, vy, vz);
    static_assert(vt == ve);
}

TEST(replace, match_head) {
    constexpr auto vx1 = "hello"_ssv;
    constexpr auto vx2 = " world"_ssv;
    constexpr auto vx = vx1 + vx2;
    constexpr auto vy = vx1;
    constexpr auto vz = "privet"_ssv;

    constexpr auto ve = vz + vx2;  // expected

    constexpr auto vt = ss::replace(vx, vy, vz);
    static_assert(vt == ve);
}

TEST(replace, match_tail) {
    constexpr auto vx1 = "hello "_ssv;
    constexpr auto vx2 = "world"_ssv;
    constexpr auto vx = vx1 + vx2;
    constexpr auto vy = vx2;
    constexpr auto vz = "mir"_ssv;

    constexpr auto ve = vx1 + vz;  // expected

    constexpr auto vt = ss::replace(vx, vy, vz);
    static_assert(vt == ve);
}

TEST(replace, match_middle) {
    constexpr auto vx1 = "hello "_ssv;
    constexpr auto vx2 = "this"_ssv;
    constexpr auto vx3 = " world"_ssv;
    constexpr auto vx = vx1 + vx2 + vx3;
    constexpr auto vy = vx2;
    constexpr auto vz = "other"_ssv;

    constexpr auto ve = vx1 + vz + vx3;  // expected

    constexpr auto vt = ss::replace(vx, vy, vz);
    static_assert(vt == ve);
}

TEST(replace, match_first_occurence) {
    constexpr auto vx1 = "hello "_ssv;
    constexpr auto vx2 = "this"_ssv;
    constexpr auto vx3 = " world and "_ssv;
    constexpr auto vx4 = " universe"_ssv;
    constexpr auto vx = vx1 + vx2 + vx3 + vx2 + vx4;
    constexpr auto vy = vx2;
    constexpr auto vz = "other"_ssv;

    constexpr auto ve = vx1 + vz + vx3 + vx2 + vx4;  // expected

    constexpr auto vt = ss::replace(vx, vy, vz);
    static_assert(vt == ve);
}
