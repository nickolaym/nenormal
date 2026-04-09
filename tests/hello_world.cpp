#include <gtest/gtest.h>

#include "nenormal/nenormal.h"

TEST(HelloWorld, HelloWorld) {
    constexpr auto text = CTSTR("hello, world!");
    constexpr auto program = RULES(
        RULE("hello", "privet"),
        RULE("world", "mir")
    );
    constexpr auto result = rule_loop<program>{}(text);
    std::cout << text << " -> " << result << std::endl;
    static_assert(result == CTSTR("privet, mir!"));
}
