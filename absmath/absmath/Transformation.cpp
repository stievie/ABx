#include "stdafx.h"
#include "Transformation.h"
#include "Matrix4.h"

namespace Math {

DirectX::XMMATRIX Transformation::GetMatrix() const
{
    const DirectX::XMVECTOR vZero = { 0 };
    const DirectX::XMVECTOR qId = { 0.0f, 0.0f, 0.0f, 1.0f };

    Quaternion rot = GetQuaternion();
    DirectX::XMVECTOR scale = XMVectorSet(scale_.x_, scale_.y_, scale_.z_, 0.0f);
    DirectX::XMVECTOR rotation = XMVectorSet(rot.x_, rot.y_, rot.z_, rot.w_);
    DirectX::XMVECTOR position = XMVectorSet(position_.x_, position_.y_, position_.z_, 0.0f);
    return XMMatrixTransformation(vZero, qId, scale, vZero, rotation, position);
}

}
