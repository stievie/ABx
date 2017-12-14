#include "stdafx.h"
#include "Transformation.h"
#include "Matrix4.h"

namespace Math {

void Transformation::LookAt(const Vector3& position, const Vector3& up)
{
    rotation_ = Matrix4::FromLookAt(position_, position, up).Rotation();
}

void Transformation::LookAt(const Vector3& position)
{
    LookAt(position, Vector3::UnitY);
}

DirectX::XMMATRIX Transformation::GetMatrix() const
{
    const DirectX::XMVECTOR vZero = { 0 };
    const DirectX::XMVECTOR qId = { 0.0f, 0.0f, 0.0f, 1.0f };

    DirectX::XMVECTOR scale = XMVectorSet(scale_.x_, scale_.y_, scale_.z_, 0.0f);
    DirectX::XMVECTOR rotation = XMVectorSet(rotation_.x_, rotation_.y_, rotation_.z_, rotation_.w_);
    DirectX::XMVECTOR position = XMVectorSet(position_.x_, position_.y_, position_.z_, 0.0f);
    return XMMatrixTransformation(vZero, qId, scale, vZero, rotation, position);
}

}
