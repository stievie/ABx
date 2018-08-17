#pragma once

#include "Vector3.h"
#include "Quaternion.h"
#include <DirectXMath.h>

namespace Math {

class Transformation
{
public:
    Vector3 position_;
    Vector3 scale_;
    /// Angle in rad Y axis
    float rotation_;

    Transformation() :
        position_(Vector3(0.0f, 0.0f, 0.0f)),
        scale_(Vector3(1.0f, 1.0f, 1.0f)),
        rotation_(0.0f)
    {}
    Transformation(const Vector3& pos, const Vector3& scale, float rot) :
        position_(pos),
        scale_(scale),
        rotation_(rot)
    {}
    Transformation(const Vector3& pos, const Vector3& scale, const Quaternion& rot) :
        position_(pos),
        scale_(scale)
    {
        rotation_ = rot.EulerAngles().y_;
    }

    ~Transformation() {};

    Quaternion GetQuaternion() const
    {
        return Quaternion::FromAxisAngle(Vector3::UnitY, rotation_);
    }

    DirectX::XMMATRIX GetMatrix() const;
};

}
