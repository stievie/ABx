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

#include <absmath/Matrix4.h>

TEST_CASE("Matrix4 Construct")
{
    Math::Matrix4 mat;
    XMath::XMMATRIX xmat = (XMath::XMMATRIX)mat;
    Math::Matrix4 mat2(xmat);
    REQUIRE(mat.Translation() == mat2.Translation());
    REQUIRE(mat.Scaling() == mat2.Scaling());
    REQUIRE(mat.Rotation() == mat2.Rotation());
}

TEST_CASE("Matrix4 FromQuaternion")
{
    Math::Quaternion q = Math::Quaternion::FromAxisAngle(Math::Vector3::UnitY, Math::M_PIF);
    Math::Matrix4 m1 = Math::Matrix4::FromQuaternion(q);
    Math::Matrix4 m2;
    m2.SetRotation(q.Conjugate());
    for (size_t i = 0; i < 16; ++i)
    {
        INFO(i)
        REQUIRE(m1.m_[i] == Approx(m2.m_[i]));
    }
}
