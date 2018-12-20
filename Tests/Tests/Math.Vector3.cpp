#include <catch.hpp>

#include "Vector3.h"

TEST_CASE("Vector3 Construct", "[vector3_construct]")
{
    SECTION("Parse from string")
    {
        Math::Vector3 vec("1.0 2.0 1.0");
        REQUIRE(vec.x_ == 1.0f);
        REQUIRE(vec.y_ == 2.0f);
        REQUIRE(vec.z_ == 1.0f);
    }
}

TEST_CASE("Vector3 Compare", "[vector3_compare]")
{
    SECTION("Compare equality")
    {
        Math::Vector3 vec1(1.0f, 2.0f, 1.0f);
        Math::Vector3 vec2(1.0f, 2.0f, 1.0f);
        REQUIRE(vec1 == vec2);
    }
    SECTION("Compare inequality")
    {
        Math::Vector3 vec1(1.0f, 2.0f, 1.0f);
        Math::Vector3 vec2(1.0f, 1.0f, 1.0f);
        REQUIRE(vec1 != vec2);
    }
}

TEST_CASE("Vector3 Arithmetic", "[vector3_arithmetic]")
{
    SECTION("Negate")
    {
        Math::Vector3 vec1(1.0f, 2.0f, 1.0f);
        Math::Vector3 vec2 = -vec1;
        REQUIRE(vec1 == -vec2);
    }
    SECTION("Increment +=")
    {
        Math::Vector3 vec1(1.0f, 2.0f, 1.0f);
        Math::Vector3 vec2(1.0f, 1.0f, 1.0f);
        vec2 += vec1;
        REQUIRE(vec2 == Math::Vector3(2.0f, 3.0f, 2.0f));
    }
}