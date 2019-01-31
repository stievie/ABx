#include "stdafx.h"
#include <catch.hpp>

#include "BoundingBox.h"
#include "Matrix4.h"
#include "Transformation.h"

TEST_CASE("BoundingBox Basic", "[boundingbox]")
{
    SECTION("Size")
    {
        Math::BoundingBox bb(-2.0f, 2.0f);
        Math::Vector3 size = bb.Size();
        REQUIRE(size.x_ == 4.0f);
        REQUIRE(size.y_ == 4.0f);
        REQUIRE(size.z_ == 4.0f);
    }
    SECTION("Center")
    {
        Math::BoundingBox bb(-2.0f, 2.0f);
        Math::Vector3 center = bb.Center();
        REQUIRE(center.x_ == 0.0f);
        REQUIRE(center.y_ == 0.0f);
        REQUIRE(center.z_ == 0.0f);
    }
    SECTION("Extends")
    {
        Math::BoundingBox bb(-2.0f, 2.0f);
        Math::Vector3 extends = bb.Extends();
        REQUIRE(extends.x_ == 2.0f);
        REQUIRE(extends.y_ == 2.0f);
        REQUIRE(extends.z_ == 2.0f);
    }

}

TEST_CASE("BoundingBox Transform", "[boundingbox]")
{
    SECTION("Scale")
    {
        Math::BoundingBox bb(-2.0f, 2.0f);
        Math::Matrix4 trans = Math::Matrix4::FromScale(Math::Vector3(1.0f, 0.5f, 1.0f));
        Math::BoundingBox transformed = bb.Transformed(trans);
        REQUIRE(transformed.min_.x_ == -2.0f);
        REQUIRE(transformed.min_.y_ == -1.0f);
        REQUIRE(transformed.min_.z_ == -2.0f);
        REQUIRE(transformed.max_.x_ == 2.0f);
        REQUIRE(transformed.max_.y_ == 1.0f);
        REQUIRE(transformed.max_.z_ == 2.0f);
    }
    SECTION("Translation")
    {
        Math::BoundingBox bb(-2.0f, 2.0f);
        Math::Matrix4 trans = Math::Matrix4::FromTranslation(Math::Vector3(1.0f, 1.0f, 1.0f));
        Math::BoundingBox transformed = bb.Transformed(trans);
        REQUIRE(transformed.min_.x_ == -1.0f);
        REQUIRE(transformed.min_.y_ == -1.0f);
        REQUIRE(transformed.min_.z_ == -1.0f);
        REQUIRE(transformed.max_.x_ == 3.0f);
        REQUIRE(transformed.max_.y_ == 3.0f);
        REQUIRE(transformed.max_.z_ == 3.0f);
    }

}

TEST_CASE("BoundingBox Collisions", "[boundingbox]")
{
    SECTION("BoundingBox inside")
    {
        Math::BoundingBox bb1(-2.0f, 2.0f);
        Math::BoundingBox bb2(-1.0f, 1.0f);
        Math::Vector3 move;
        REQUIRE(bb1.Collides(bb2, move));
        REQUIRE(move.x_ == 3.0f);
        REQUIRE(move.y_ == 0.0f);
        REQUIRE(move.z_ == 0.0f);
    }
    SECTION("BoundingBox outside")
    {
        Math::BoundingBox bb1(-4.0f, -1.0f);
        Math::BoundingBox bb2(0.0f, 2.0f);
        Math::Vector3 move;
        REQUIRE(!bb1.Collides(bb2, move));
        REQUIRE(move.x_ == 0.0f);
        REQUIRE(move.y_ == 0.0f);
        REQUIRE(move.z_ == 0.0f);
    }
    SECTION("BoundingBox oriented inside")
    {
        Math::BoundingBox bb1(-2.0f, 2.0f);
        Math::BoundingBox bb2(-1.0f, 1.0f);
        // 90 Deg
        bb2.orientation_ = Math::Quaternion::FromAxisAngle(Math::Vector3::UnitY, 1.570796f);
        Math::Vector3 move;
        REQUIRE(bb1.Collides(bb2, move));
    }
}

TEST_CASE("BoundingBox moving Collisions", "[boundingbox]")
{
    SECTION("BoundingBox oriented moving")
    {
        // First is moving, second is oriented
        Math::BoundingBox bb1(-1.0f, 1.0f);
        Math::BoundingBox bb2(-2.0f, 2.0f);
        // 90 Deg
        bb2.orientation_ = Math::Quaternion::FromAxisAngle(Math::Vector3::UnitY, 1.570796f);
        Math::Transformation trans;
        for (unsigned i = 0; i < 5; ++i)
        {
            Math::Vector3 move;
            trans.position_.x_ += (float)i;
            Math::BoundingBox bbTrans = bb1.Transformed(trans.GetMatrix());
            INFO("i = " << i);
            if (i < 3)
                REQUIRE(bbTrans.Collides(bb2, move));
            else
                REQUIRE(!bbTrans.Collides(bb2, move));
        }
    }
    SECTION("BoundingBox oriented rotating")
    {
        // First is moving, second is oriented
        Math::BoundingBox bb1(2.0f, 4.0f);
        Math::BoundingBox bb2(-2.0f, 2.0f);
        Math::Transformation trans;
        for (unsigned i = 0; i < 4; ++i)
        {
            Math::Vector3 move;
            bb1.orientation_ = Math::Quaternion::FromAxisAngle(Math::Vector3::UnitY, (float)i);

            INFO("i = " << i);
            if (i < 1)
                REQUIRE(bb1.Collides(bb2, move));
            else
                REQUIRE(!bb1.Collides(bb2, move));
        }
    }
}

TEST_CASE("XMath::BoundingBox Collisions", "[boundingbox]")
{
    SECTION("BoundingBox inside")
    {
        Math::BoundingBox bb1(-2.0f, 2.0f);
        Math::BoundingBox bb2(-1.0f, 1.0f);
        XMath::BoundingBox xbb1 = (XMath::BoundingBox)bb1;
        XMath::BoundingBox xbb2 = (XMath::BoundingBox)bb2;
        REQUIRE(xbb1.Intersects(xbb2));
    }
    SECTION("BoundingBox outside")
    {
        Math::BoundingBox bb1(-4.0f, -1.0f);
        Math::BoundingBox bb2(0.0f, 2.0f);
        XMath::BoundingBox xbb1 = (XMath::BoundingBox)bb1;
        XMath::BoundingBox xbb2 = (XMath::BoundingBox)bb2;
        REQUIRE(!xbb1.Intersects(xbb2));
    }
}

TEST_CASE("BoundingBox Intersections", "[boundingbox]")
{
    SECTION("Vector3 inside")
    {
        Math::BoundingBox bb1(-2.0f, 2.0f);
        Math::Vector3 inside(1.0f, 1.0f, 1.0f);
        Math::Intersection in = bb1.IsInside(inside);
        REQUIRE(in == Math::INSIDE);
    }
    SECTION("Vector3 outside")
    {
        Math::BoundingBox bb1(-2.0f, 2.0f);
        Math::Vector3 inside(3.0f, 1.0f, 1.0f);
        Math::Intersection in = bb1.IsInside(inside);
        REQUIRE(in == Math::OUTSIDE);
    }
}
