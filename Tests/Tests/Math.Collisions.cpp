/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include <catch.hpp>

#include <absmath/Quaternion.h>

TEST_CASE("Collisions")
{
    SECTION("AABB vs OBB -45Deg")
    {

        XMath::BoundingBox abb({ -3.5f, 0.0f, -3.5f }, { 0.5f, 0.5f, 0.5f });
        // -45Deg Y axis
        XMath::XMFLOAT4 orientation = { 0.000000000f, -0.382683456f, 0.000000000f, 0.923879504f };
        //XMath::XMFLOAT4 o3 = Math::Quaternion::FromAxisAngle(Math::Vector3::UnitY, Math::DegToRad(-45.0f));

        XMath::BoundingOrientedBox obb({ 0.0f, 0.0f, 0.0f }, { 4.0f, 0.5f, 2.0f }, orientation);
        while (!abb.Intersects(obb))
        {
            abb.Center.x += 0.5f;
            abb.Center.z += 0.5f;
        }
        // Must intersect @ x = -3.0, z = -3.0
        REQUIRE(abb.Intersects(obb));
        REQUIRE(abb.Center.x == Approx(-3.0f));
        REQUIRE(abb.Center.z == Approx(-3.0f));

        XMath::BoundingBox abb2({ -2.5f, 0.0f, 2.5f }, { 0.5f, 0.5f, 0.5f });
        REQUIRE(!abb2.Intersects(obb));
    }

    SECTION("AABB vs OBB 45Deg")
    {
        // Works as expected. Maybe I shouldn't have negative orientation?
        XMath::BoundingBox abb({ -3.5f, 0.0f, -3.5f }, { 0.5f, 0.5f, 0.5f });
        // 45Deg Y
        XMath::XMFLOAT4 orientation = { 0.000000000f, 0.382683456f, 0.000000000f, 0.923879504f };

        XMath::BoundingOrientedBox obb({ 0.0f, 0.0f, 0.0f }, { 4.0f, 0.5f, 2.0f }, orientation);
        while (!abb.Intersects(obb))
        {
            abb.Center.x += 0.5f;
            abb.Center.z += 0.5f;
        }
        // Must intersect @ x = -1.5, z = -1.5
        REQUIRE(abb.Intersects(obb));
        REQUIRE(abb.Center.x == Approx(-1.5f));
        REQUIRE(abb.Center.z == Approx(-1.5f));

        XMath::BoundingBox abb2({ -2.5f, 0.0f, 2.5f }, { 0.5f, 0.5f, 0.5f });
        REQUIRE(abb2.Intersects(obb));
        while (abb2.Intersects(obb))
        {
            abb2.Center.x -= 0.5f;
            abb2.Center.z += 0.5f;
        }
        // Must intersect @ x = -2.0, z = 1.5
        REQUIRE(!abb2.Intersects(obb));
        REQUIRE(abb2.Center.x == Approx(-3.5f));
        REQUIRE(abb2.Center.z == Approx(3.5f));
    }

    SECTION("AABB vs AABB")
    {

        XMath::BoundingBox abb1({ -3.0f, 0.0f, -3.0f }, { 0.5f, 0.5f, 0.5f });

        XMath::BoundingBox abb2({ 0.0f, 0.0f, 0.0f }, { 4.0f, 0.5f, 2.0f });
        while (!abb1.Intersects(abb2))
        {
            abb1.Center.x += 0.5f;
            abb1.Center.z += 0.5f;
        }
        REQUIRE(abb1.Intersects(abb2));
    }
}
