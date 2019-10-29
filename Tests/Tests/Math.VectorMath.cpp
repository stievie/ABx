#include "stdafx.h"
#include <catch.hpp>

#include "VectorMath.h"

TEST_CASE("IsPointInTriangle")
{
    Math::Vector3 p1{ 0.0f, 0.0f, -1.0f };
    Math::Vector3 p2{ 1.0f, 0.0f, 0.0f };
    Math::Vector3 p3{ 0.0f, 0.0f, 1.0f };

    Math::Vector3 pt1{ 0.0f, 0.0f, 0.0f };
    bool res = Math::IsPointInTriangle(pt1, p1, p2, p3);
    REQUIRE(res);

    Math::Vector3 pt2{ 5.0f, 0.0f, 0.0f };
    bool res2 = Math::IsPointInTriangle(pt2, p1, p2, p3);
    REQUIRE(!res2);
}
