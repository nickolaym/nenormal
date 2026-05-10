#include <gtest/gtest.h>

#include "nenormal/nenormal.h"

TEST(HelloWorld, HelloWorld) {
    constexpr auto text = CTSTR("hello, world!");
    constexpr auto program = RULES(
        RULE("hello", "privet"),
        RULE("world", "mir")
    );
    constexpr auto machine = MACHINE(program);
    constexpr auto result = machine(text);
    std::cout << text << " -> " << result << std::endl;
    static_assert(result == CTSTR("privet, mir!"));
}
