#include "stdafx.h"
#include "Transformation.h"
#include "Matrix4.h"

namespace Math {

XMath::XMMATRIX Transformation::GetMatrix() const
{
    const XMath::XMVECTOR vZero = { 0 };
    const XMath::XMVECTOR qId = { 0.0f, 0.0f, 0.0f, 1.0f };

    Quaternion rot = GetQuaternion();
    XMath::XMVECTOR scale = XMath::XMVectorSet(scale_.x_, scale_.y_, scale_.z_, 0.0f);
    XMath::XMVECTOR rotation = XMath::XMVectorSet(rot.x_, rot.y_, rot.z_, rot.w_);
    XMath::XMVECTOR position = XMath::XMVectorSet(position_.x_, position_.y_, position_.z_, 0.0f);
    return XMath::XMMatrixTransformation(vZero, qId, scale, vZero, rotation, position);
}

}
