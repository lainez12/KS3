#include "utils.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Utils: dummy test (mul2)", "[utils]")
{
    SECTION("Positive numbers")
    {
        REQUIRE(Kub3::Utils::mul2(2) == 4);
        REQUIRE(Kub3::Utils::mul2(10) == 20);
    }

    SECTION("Zero and negative numbers")
    {
        REQUIRE(Kub3::Utils::mul2(0) == 0);
        REQUIRE(Kub3::Utils::mul2(-5) == -10);
    }
}
