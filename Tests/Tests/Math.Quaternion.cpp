#include "stdafx.h"
#include <catch.hpp>

#include "Quaternion.h"
#include "Matrix4.h"

// https://github.com/gareth-cross/quat/blob/master/test/quaternion_test.cpp

TEST_CASE("Quaternion")
{
    SECTION("Construct")
    {
        Math::Quaternion quat(0.0f, 1.0f, 2.0f, 3.0f);
        REQUIRE(quat.w_ == 0.0f);
        REQUIRE(quat.x_ == 1.0f);
        REQUIRE(quat.y_ == 2.0f);
        REQUIRE(quat.z_ == 3.0f);
    }
    SECTION("From String")
    {
        // 45 Deg
        Math::Quaternion quat("0.92388 0 0.382683 0");
        REQUIRE(fabs(quat.w_ - 0.923879504f) < 0.00001f);
        REQUIRE(quat.x_ == 0.0f);
        REQUIRE(fabs(quat.y_ - 0.382683456f) < 0.00001f);
        REQUIRE(quat.z_ == 0.0f);
    }
    SECTION("From AxisAngle")
    {
        Math::Quaternion quat = Math::Quaternion::FromAxisAngle(Math::Vector3::UnitY, Math::DegToRad(45.0f));
        REQUIRE(fabs(quat.w_ - 0.923879504f) < 0.00001f);
        REQUIRE(quat.x_ == 0.0f);
        REQUIRE(fabs(quat.y_ - 0.382683456f) < 0.00001f);
        REQUIRE(quat.z_ == 0.0f);
    }
    SECTION("Euler")
    {
        Math::Quaternion quat(0.92388f, 0.0f, 0.382683f, 0.0f);
        Math::Quaternion quat2(0.0f, Math::DegToRad(45.0f), 0.0f);
        Math::Vector3 euler = quat.EulerAngles();
        REQUIRE(euler.x_ == 0.0f);
        REQUIRE(fabs(Math::RadToDeg(euler.y_) - 45.0f) < 0.00006f);
        REQUIRE(euler.z_ == 0.0f);
    }
    SECTION("To AxisAngle")
    {
        // 45 Deg
        Math::Quaternion quat(0.92388f, 0.0f, 0.382683f, 0.0f);
        Math::Vector4 aa = quat.AxisAngle();
        REQUIRE(fabs(aa.y_ * aa.w_ - Math::DegToRad(45.0f)) < 0.0001f);
        REQUIRE(aa.x_ == 0.0f);
        REQUIRE(aa.z_ == 0.0f);
    }

    SECTION("Multiply")
    {
        Math::Quaternion quat = Math::Quaternion::FromAxisAngle(Math::Vector3::UnitX, float(M_PI));
        Math::Vector3 v = Math::Matrix4::FromQuaternion(quat) * Math::Vector3::UnitY;

//        REQUIRE(v.Equals(Math::Vector3::UnitY));
    }
    SECTION("Conjugate")
    {
        Math::Quaternion quat(1.0f, 1.0f, 1.0f, 1.0f);
        Math::Quaternion con = quat.Conjugate();
        REQUIRE(con.Equals(Math::Quaternion(1.0f, -1.0f, -1.0f, -1.0f)));
    }
    SECTION("Inverse")
    {
        Math::Quaternion quat(1.0f, 1.0f, 1.0f, 1.0f);
        Math::Quaternion inv = quat.Inverse();
        REQUIRE(inv.Equals(Math::Quaternion(0.25f, -0.25f, -0.25f, -0.25f)));
    }
}
