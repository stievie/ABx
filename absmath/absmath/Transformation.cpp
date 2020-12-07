/**
 * Copyright 2017-2020 Stefan Ascher
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

#include "Transformation.h"

namespace Math {

Matrix4 Transformation::GetMatrix() const
{
    return GetMatrix(oriention_);
}

Matrix4 Transformation::GetMatrix(const Quaternion& rot) const
{
#if defined(HAVE_DIRECTX_MATH)
    static const XMath::XMVECTOR vZero = { 0 };
    static const XMath::XMVECTOR qId = { 0.0f, 0.0f, 0.0f, 1.0f };
    const XMath::XMVECTOR scale = XMath::XMVectorSet(scale_.x_, scale_.y_, scale_.z_, 0.0f);
    const XMath::XMVECTOR rotation = XMath::XMVectorSet(rot.x_, rot.y_, rot.z_, rot.w_);
    const XMath::XMVECTOR position = XMath::XMVectorSet(position_.x_, position_.y_, position_.z_, 0.0f);
    return XMath::XMMatrixTransformation(vZero, qId, scale, vZero, rotation, position);
#else
    return Matrix4(position_, rot.Conjugate(), scale_);
#endif
}

void Transformation::Move(float speed, const Vector3& amount)
{
    // new position = position + direction * speed (where speed = amount * speed)

    // It's as easy as:
    // 1. Create a matrix from the rotation,
    // 2. multiply this matrix with the moving vector and
    // 3. add the resulting vector to the current position
#if defined(HAVE_DIRECTX_MATH)
    const XMath::XMMATRIX m = XMath::XMMatrixRotationAxis(Math::Vector3::UnitY, -GetYRotation());
    const Vector3 a = amount * speed;
    const XMath::XMVECTOR v = XMath::XMVector3Transform(a, m);
    position_.x_ += XMath::XMVectorGetX(v);
    position_.y_ += XMath::XMVectorGetY(v);
    position_.z_ += XMath::XMVectorGetZ(v);
#else
    const Matrix4 m = Math::Matrix4::FromAxisAngle(Math::Vector3::UnitY, -GetYRotation());
    const Vector3 a = amount * speed;
    const Vector3 v = m * a;
    position_ += v;
#endif
}

void Transformation::Turn(float yAngle)
{
    float ang = GetYRotation();
    ang += yAngle;
    SetYRotation(ang);
}

float Transformation::GetYRotation() const
{
    return Math::NormalizedAngle(oriention_.EulerAngles().y_);
}

void Transformation::SetYRotation(float rad)
{
    NormalizeAngle(rad);
    const Math::Vector3 euler = oriention_.EulerAngles();
    oriention_ = Quaternion(euler.x_, rad, euler.y_);
}

void Transformation::LookAt(const Vector3& lookAt, const Vector3& up /* = Math::Vector3::UnitY */)
{
    const Matrix4 mat = Matrix4::FromLookAt(lookAt, position_, up);
    oriention_ = mat.Rotation();
}

}
