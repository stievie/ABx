#include "stdafx.h"
#include <catch.hpp>

#include "Transformation.h"
#include "Matrix4.h"

TEST_CASE("Transformation Basic")
{
    SECTION("Construct")
    {
        Math::Transformation trans;
        REQUIRE(trans.position_.Equals(Math::Vector3::Zero));
        REQUIRE(trans.scale_.Equals(Math::Vector3::One));
        REQUIRE(trans.oriention_.Equals(Math::Quaternion::Identity));
    }
    SECTION("Rotation")
    {
        Math::Transformation trans;
        trans.SetYRotation(Math::M_PIHALF);
        REQUIRE(trans.GetYRotation() == Approx(Math::M_PIHALF));
    }
    SECTION("GetMatrix")
    {
        Math::Transformation trans(Math::Vector3::One, Math::Vector3(0.5f, 0.5f, 0.5f), Math::M_PIHALF);
        Math::Matrix4 matrix = trans.GetMatrix();
        Math::Vector3 transl;
        Math::Vector3 scaling;
        Math::Quaternion orient;
        matrix.Decompose(&scaling, &orient, &transl);
        INFO(transl.ToString());
        REQUIRE(transl.Equals(Math::Vector3::One));
        REQUIRE(scaling.Equals(Math::Vector3(0.5f, 0.5f, 0.5f)));
        Math::Quaternion orient2 = Math::Quaternion::FromAxisAngle(Math::Vector3::UnitY, Math::M_PIHALF);
        REQUIRE(orient.Equals(orient2));
    }
}

TEST_CASE("Transformation Move")
{
    float speed = 0.5f;
    const Math::Vector3& amount = Math::Vector3::UnitZ;
    SECTION("Move")
    {
        Math::Transformation trans(Math::Vector3::One, Math::Vector3(0.5f, 0.5f, 0.5f), Math::M_PIHALF);
        trans.Move(speed, amount);
        REQUIRE(trans.position_.x_ == Approx(0.5f));
        REQUIRE(trans.position_.y_ == Approx(1.0f));
        REQUIRE(trans.position_.z_ == Approx(1.0f));
    }
    SECTION("Move Internal")
    {
        Math::Transformation trans(Math::Vector3::One, Math::Vector3(0.5f, 0.5f, 0.5f), Math::M_PIHALF);
        Math::Matrix4 m = Math::Matrix4::FromQuaternion(trans.oriention_.Inverse());
        Math::Vector3 a = amount * speed;
        Math::Vector3 v = m * a;
        trans.position_ += v;
        REQUIRE(trans.position_.x_ == Approx(0.5f));
        REQUIRE(trans.position_.y_ == Approx(1.0f));
        REQUIRE(trans.position_.z_ == Approx(1.0f));
    }
    SECTION("Move XMath")
    {
        Math::Transformation trans(Math::Vector3::One, Math::Vector3(0.5f, 0.5f, 0.5f), Math::M_PIHALF);
        XMath::XMMATRIX m = XMath::XMMatrixRotationAxis(Math::Vector3::UnitY, -trans.GetYRotation());
        Math::Vector3 a = amount * speed;
        XMath::XMVECTOR v = XMath::XMVector3Transform(a, m);
        trans.position_.x_ += XMath::XMVectorGetX(v);
        trans.position_.y_ += XMath::XMVectorGetY(v);
        trans.position_.z_ += XMath::XMVectorGetZ(v);
        REQUIRE(trans.position_.x_ == Approx(0.5f));
        REQUIRE(trans.position_.y_ == Approx(1.0f));
        REQUIRE(trans.position_.z_ == Approx(1.0f));
    }
}
