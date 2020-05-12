#include "stdafx.h"
#include <catch.hpp>

#include <sa/Bits.h>

static const uint32_t BIT1 = 1;
static const uint32_t BIT2 = 1 << 1;
static const uint32_t BIT3 = 1 << 2;
static const uint32_t BIT4 = 1 << 3;
static const uint32_t BIT5 = 1 << 4;
static const uint32_t BIT6 = 1 << 5;
static const uint32_t BIT7 = 1 << 6;
static const uint32_t BIT8 = 1 << 8;

TEST_CASE("Bits is_set")
{
    uint32_t bit_set = BIT2 | BIT4;
    REQUIRE(sa::bits::is_set(bit_set, BIT4));
    REQUIRE(sa::bits::is_set(bit_set, BIT2));
    REQUIRE(!sa::bits::is_set(bit_set, BIT5));
}

TEST_CASE("Bits un_set")
{
    uint32_t bit_set = BIT2 | BIT4;
    REQUIRE(sa::bits::is_set(bit_set, BIT4));
    REQUIRE(sa::bits::is_set(bit_set, BIT2));
    sa::bits::un_set(bit_set, BIT4);
    REQUIRE(!sa::bits::is_set(bit_set, BIT4));
    REQUIRE(sa::bits::is_set(bit_set, BIT2));
}

TEST_CASE("Bits is_set_any")
{
    uint32_t bit_set = BIT2 | BIT4;
    REQUIRE(sa::bits::is_any_set(bit_set, BIT4 | BIT5));
    REQUIRE(!sa::bits::is_any_set(bit_set, BIT1 | BIT5));
}

TEST_CASE("Bits flip")
{
    uint32_t bit_set = BIT2 | BIT4;
    REQUIRE(sa::bits::to_string(bit_set) == "00000000000000000000000000001010");
    sa::bits::flip(bit_set);
    REQUIRE(sa::bits::to_string(bit_set) == "11111111111111111111111111110101");
}

TEST_CASE("Bits count")
{
    REQUIRE(sa::bits::count<uint32_t>() == 32);
    REQUIRE(sa::bits::count<uint64_t>() == 64);
}

TEST_CASE("Bits to_string")
{
    uint32_t bit_set = BIT2 | BIT4 | BIT8;
    REQUIRE(sa::bits::to_string(bit_set) == "00000000000000000000000100001010");
}
