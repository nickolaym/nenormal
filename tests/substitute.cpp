#include "nenormal/substitute.h"
#include <gtest/gtest.h>

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
