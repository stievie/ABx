#include "stdafx.h"
#include "Transformation.h"
#include "Matrix4.h"

namespace Math {

XMath::XMMATRIX Transformation::GetMatrix() const
{
    return GetMatrix(oriention_);
}

XMath::XMMATRIX Transformation::GetMatrix(const Quaternion& rot) const
{
    static const XMath::XMVECTOR vZero = { 0 };
    static const XMath::XMVECTOR qId = { 0.0f, 0.0f, 0.0f, 1.0f };
    const XMath::XMVECTOR scale = XMath::XMVectorSet(scale_.x_, scale_.y_, scale_.z_, 0.0f);
    const XMath::XMVECTOR rotation = XMath::XMVectorSet(rot.x_, rot.y_, rot.z_, rot.w_);
    const XMath::XMVECTOR position = XMath::XMVectorSet(position_.x_, position_.y_, position_.z_, 0.0f);
    return XMath::XMMatrixTransformation(vZero, qId, scale, vZero, rotation, position);
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
    const Matrix4 m = Math::Matrix4::FromQuaternion(oriention_.Inverse());
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

void Transformation::LookAt(const Vector3& lookAt, const Vector3& up)
{
    const Matrix4 mat = Matrix4::FromLookAt(lookAt, position_, up);
    oriention_ = mat.Rotation();
}

}
