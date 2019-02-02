#include "stdafx.h"
#include <catch.hpp>

#include "Quaternion.h"
#include "Matrix4.h"

// https://github.com/gareth-cross/quat/blob/master/test/quaternion_test.cpp

TEST_CASE("Quaternion Construct")
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
        REQUIRE(quat.w_ == Approx(0.923879504f));
        REQUIRE(quat.x_ == 0.0f);
        REQUIRE(quat.y_ == Approx(0.382683456f));
        REQUIRE(quat.z_ == 0.0f);
    }
    SECTION("From AxisAngle")
    {
        Math::Quaternion quat = Math::Quaternion::FromAxisAngle(Math::Vector3::UnitY, Math::DegToRad(45.0f));
        REQUIRE(quat.w_ == Approx(0.923879504f));
        REQUIRE(quat.x_ == 0.0f);
        REQUIRE(quat.y_ == Approx(0.382683456f));
        REQUIRE(quat.z_ == 0.0f);
    }
    SECTION("Euler")
    {
        Math::Quaternion quat(0.92388f, 0.0f, 0.382683f, 0.0f);
        Math::Quaternion quat2(0.0f, Math::DegToRad(45.0f), 0.0f);
        Math::Vector3 euler = quat.EulerAngles();
        REQUIRE(euler.x_ == 0.0f);
        REQUIRE(Math::RadToDeg(euler.y_) == Approx(45.0f));
        REQUIRE(euler.z_ == 0.0f);
    }
    SECTION("To AxisAngle")
    {
        // 45 Deg
        Math::Quaternion quat(0.92388f, 0.0f, 0.382683f, 0.0f);
        Math::Vector4 aa = quat.AxisAngle();
        REQUIRE(aa.y_ * aa.w_ == Approx(Math::DegToRad(45.0f)));
        REQUIRE(aa.x_ == 0.0f);
        REQUIRE(aa.z_ == 0.0f);
    }
}

TEST_CASE("Quaternion Operations")
{
    SECTION("Addition")
    {
        Math::Quaternion quat1(1.0f, 2.0f, 3.0f, 4.0f);
        Math::Quaternion quat2(1.0f, -2.0f, 3.0f, -4.0f);
        Math::Quaternion add = quat1 + quat2;

        REQUIRE(add.w_ == 0.0f);
        REQUIRE(add.x_ == 6.0f);
        REQUIRE(add.y_ == 0.0f);
        REQUIRE(add.z_ == 2.0f);
    }

    SECTION("Multiply")
    {
        Math::Quaternion quat1(1 / std::sqrt(2.0f), 1.0f / std::sqrt(2.0f), 0.0f, 0.0f);
        Math::Quaternion quat2(1 / std::sqrt(2.0f), 0.0f, 1.0f / std::sqrt(2.0f), 0.0f);
        Math::Quaternion mul = quat1 * quat2;

        REQUIRE(mul.w_ == Approx(0.5f));
        REQUIRE(mul.x_ == Approx(0.5f));
        REQUIRE(mul.y_ == Approx(0.5f));
        REQUIRE(mul.z_ == Approx(0.5f));

        mul = quat1 * quat1.Conjugate();

        REQUIRE(mul.w_ == Approx(0.0f));
        REQUIRE(mul.x_ == Approx(0.0f));
        REQUIRE(mul.y_ == Approx(0.0f));
        REQUIRE(mul.z_ == Approx(1.0f));
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

TEST_CASE("Quaternion Methods")
{
    SECTION("Scaling")
    {
        Math::Quaternion quat(1.0f, 2.0f, 3.0f, 4.0f);
        quat = quat * 2.0f;
        REQUIRE(quat.w_ == Approx(4.0f));
        REQUIRE(quat.x_ == Approx(6.0f));
        REQUIRE(quat.y_ == Approx(8.0f));
        REQUIRE(quat.z_ == Approx(2.0f));

        quat = quat * 0.5f;
        REQUIRE(quat.w_ == Approx(3.0f));
        REQUIRE(quat.x_ == Approx(4.0f));
        REQUIRE(quat.y_ == Approx(1.0f));
        REQUIRE(quat.z_ == Approx(2.0f));

        quat = quat / 2.0f;
        REQUIRE(quat.w_ == Approx(2.0f));
        REQUIRE(quat.x_ == Approx(0.5f));
        REQUIRE(quat.y_ == Approx(1.0f));
        REQUIRE(quat.z_ == Approx(1.5f));
    }
    SECTION("Scaling")
    {
        Math::Quaternion q = Math::Quaternion::FromAxisAngle(Math::Vector3::UnitY, float(M_PI) / 3.0f);
        REQUIRE(q.w_ == Approx(std::sqrt(3.0f) / 2.0f));
        REQUIRE(q.x_ == Approx(0.0f));
        REQUIRE(q.y_ == Approx(0.5f));
        REQUIRE(q.z_ == Approx(0.0f));

    }
}
