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
    XMath::XMVECTOR scale = XMath::XMVectorSet(scale_.x_, scale_.y_, scale_.z_, 0.0f);
    XMath::XMVECTOR rotation = XMath::XMVectorSet(rot.x_, rot.y_, rot.z_, rot.w_);
    XMath::XMVECTOR position = XMath::XMVectorSet(position_.x_, position_.y_, position_.z_, 0.0f);
    return XMath::XMMatrixTransformation(vZero, qId, scale, vZero, rotation, position);
}

float Transformation::GetYRotation() const
{
    return oriention_.EulerAngles().y_;
}

void Transformation::SetYRotation(float rad)
{
    oriention_ = Quaternion(oriention_.x_, rad, oriention_.z_);
}

void Transformation::LookAt(const Vector3& lookAt, const Vector3& up)
{
    Matrix4 mat = Matrix4::FromLookAt(lookAt, position_, up);
    oriention_ = mat.Rotation();
}

}
