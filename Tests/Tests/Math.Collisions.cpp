#include "stdafx.h"
#include <catch.hpp>

#include "Quaternion.h"

TEST_CASE("Collisions")
{
    SECTION("AABB vs OBB -45Deg")
    {

        XMath::BoundingBox abb({ -3.5f, 0.0f, -3.5f }, { 0.5f, 0.5f, 0.5f });
        // -45Deg Y axis
        XMath::XMVECTOR orientation2 = XMath::XMQuaternionRotationRollPitchYaw(M_PI_4, 0.0f, 0.0f);
        XMath::XMFLOAT4 orientation = { 0.000000000, -0.382683456, 0.000000000, 0.923879504 };
        XMath::XMFLOAT4 o3 = Math::Quaternion::FromAxisAngle(Math::Vector3::UnitY, Math::DegToRad(-45.0f));

        XMath::BoundingOrientedBox obb({ 0.0f, 0.0f, 0.0f }, { 4.0f, 0.5f, 2.0f }, orientation);
        while (!abb.Intersects(obb))
        {
            abb.Center.x += 0.5f;
            abb.Center.z += 0.5f;
        }
        // Must intersect @ x = -3.0, z = -3.0
        REQUIRE(abb.Intersects(obb));
        REQUIRE(abb.Center.x == -3.0f);
        REQUIRE(abb.Center.z == -3.0f);

        XMath::BoundingBox abb2({ -3.0f, 0.0f, 2.5f }, { 0.5f, 0.5f, 0.5f });
        // BUG? Does not intersect here.
//        REQUIRE(!abb.Intersects(obb));
        while (!abb.Intersects(obb))
        {
            abb2.Center.x += 0.5f;
            abb2.Center.z += 0.5f;
        }
        // Must intersect @ x = -2.0, z = 1.5
        REQUIRE(abb.Intersects(obb));
//        REQUIRE(abb.Center.x == -2.0f);
//        REQUIRE(abb.Center.z == 1.5f);
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
        REQUIRE(abb.Center.x == -1.5f);
        REQUIRE(abb.Center.z == -1.5f);

        XMath::BoundingBox abb2({ -2.5f, 0.0f, 2.5f }, { 0.5f, 0.5f, 0.5f });
        REQUIRE(abb2.Intersects(obb));
        while (abb2.Intersects(obb))
        {
            abb2.Center.x -= 0.5f;
            abb2.Center.z += 0.5f;
        }
        // Must intersect @ x = -2.0, z = 1.5
        REQUIRE(!abb2.Intersects(obb));
        REQUIRE(abb2.Center.x == -3.5f);
        REQUIRE(abb2.Center.z == 3.5f);
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