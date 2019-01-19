#include "stdafx.h"
#include <catch.hpp>

#include "Sphere.h"

TEST_CASE("Sphere", "[sphere]")
{
    SECTION("Distance")
    {
        Math::Sphere sphere(Math::Vector3(0.0f, 0.0f, 0.0f), 2.0f);
        REQUIRE(sphere.Distance(Math::Vector3(0.0f, 0.0f, 5.0f)) == 3.0f);
    }
}