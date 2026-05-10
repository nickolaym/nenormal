#include "nenormal/substitute.h"
#include <gtest/gtest.h>

namespace nn {

TEST(substitute, failed) {
    static_assert(try_substitute("abc"_cts, "!"_cts, ""_cts) == nothing{});
    static_assert(try_substitute("abc"_cts, "!"_cts, "ab"_cts) == nothing{});
    static_assert(try_substitute("abc"_cts, "!"_cts, "abbc"_cts) == nothing{});
    static_assert(try_substitute("abc"_cts, "!"_cts, "bc"_cts) == nothing{});
    static_assert(try_substitute("abc"_cts, "!"_cts, "bcca"_cts) == nothing{});
}

TEST(substitute, trivial) {
    static_assert(try_substitute(""_cts, ""_cts, ""_cts) == just{""_cts});
    static_assert(try_substitute(""_cts, ""_cts, "foo"_cts) == just{"foo"_cts});
}

TEST(substitute, exact) {
    static_assert(try_substitute("abc"_cts, "defgh"_cts, "abc"_cts) == just{"defgh"_cts});
}

TEST(substitute, delete) {
    static_assert(try_substitute("abc"_cts, ""_cts, "abcdefg"_cts) == just{"defg"_cts});
    static_assert(try_substitute("abc"_cts, ""_cts, "defgabc"_cts) == just{"defg"_cts});
    static_assert(try_substitute("abc"_cts, ""_cts, "deabcfg"_cts) == just{"defg"_cts});
}

TEST(substitute, shrink) {
    static_assert(try_substitute("abc"_cts, "<>"_cts, "abcdefg"_cts) == just{"<>defg"_cts});
    static_assert(try_substitute("abc"_cts, "<>"_cts, "deabcfg"_cts) == just{"de<>fg"_cts});
    static_assert(try_substitute("abc"_cts, "<>"_cts, "defgabc"_cts) == just{"defg<>"_cts});
}

TEST(substitute, grow) {
    static_assert(try_substitute("abc"_cts, "<-->"_cts, "abcdefg"_cts) == just{"<-->defg"_cts});
    static_assert(try_substitute("abc"_cts, "<-->"_cts, "deabcfg"_cts) == just{"de<-->fg"_cts});
    static_assert(try_substitute("abc"_cts, "<-->"_cts, "defgabc"_cts) == just{"defg<-->"_cts});
}

// inplace

TEST(substitute_opt, failed) {
    EXPECT_EQ(try_substitute_opt("abc"_cts, "!"_cts, ""), std::nullopt);
    EXPECT_EQ(try_substitute_opt("abc"_cts, "!"_cts, "ab"), std::nullopt);
    EXPECT_EQ(try_substitute_opt("abc"_cts, "!"_cts, "abbc"), std::nullopt);
    EXPECT_EQ(try_substitute_opt("abc"_cts, "!"_cts, "bc"), std::nullopt);
    EXPECT_EQ(try_substitute_opt("abc"_cts, "!"_cts, "bcca"), std::nullopt);
}

TEST(substitute_opt, trivial) {
    EXPECT_EQ(try_substitute_opt(""_cts, ""_cts, ""), optional_string{""});
    EXPECT_EQ(try_substitute_opt(""_cts, ""_cts, "foo"), optional_string{"foo"});
}

TEST(substitute_opt, exact) {
    EXPECT_EQ(try_substitute_opt("abc"_cts, "defgh"_cts, "abc"), optional_string{"defgh"});
}

TEST(substitute_opt, delete) {
    EXPECT_EQ(try_substitute_opt("abc"_cts, ""_cts, "abcdefg"), optional_string{"defg"});
    EXPECT_EQ(try_substitute_opt("abc"_cts, ""_cts, "defgabc"), optional_string{"defg"});
    EXPECT_EQ(try_substitute_opt("abc"_cts, ""_cts, "deabcfg"), optional_string{"defg"});
}

TEST(substitute_opt, shrink) {
    EXPECT_EQ(try_substitute_opt("abc"_cts, "<>"_cts, "abcdefg"), optional_string{"<>defg"});
    EXPECT_EQ(try_substitute_opt("abc"_cts, "<>"_cts, "deabcfg"), optional_string{"de<>fg"});
    EXPECT_EQ(try_substitute_opt("abc"_cts, "<>"_cts, "defgabc"), optional_string{"defg<>"});
}

TEST(substitute_opt, grow) {
    EXPECT_EQ(try_substitute_opt("abc"_cts, "<-->"_cts, "abcdefg"), optional_string{"<-->defg"});
    EXPECT_EQ(try_substitute_opt("abc"_cts, "<-->"_cts, "deabcfg"), optional_string{"de<-->fg"});
    EXPECT_EQ(try_substitute_opt("abc"_cts, "<-->"_cts, "defgabc"), optional_string{"defg<-->"});
}

} // namespace nn
