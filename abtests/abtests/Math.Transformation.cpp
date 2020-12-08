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


#include <catch.hpp>

#include <absmath/Transformation.h>
#include <absmath/Matrix4.h>

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

TEST_CASE("Transformation Turn")
{
    Math::Transformation trans(Math::Vector3::One, Math::Vector3(0.5f, 0.5f, 0.5f), Math::M_PIHALF);
    trans.Turn(Math::M_PIHALF);
    REQUIRE(trans.GetYRotation() == Approx(Math::M_PIF));
}

TEST_CASE("Transformation Matrix")
{
    Math::Transformation trans(Math::Vector3::One, Math::Vector3(0.5f, 0.5f, 0.5f), Math::M_PIHALF);
    Math::Matrix4 matrix1 = trans.GetMatrix();
    Math::Matrix4 matrix2(trans.position_, trans.oriention_, trans.scale_);
    for (size_t i = 0; i < 16; ++i)
    {
        INFO(i)
        REQUIRE(matrix1.m_[i] == Approx(matrix2.m_[i]));
    }
}
