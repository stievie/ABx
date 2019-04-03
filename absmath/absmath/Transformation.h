#pragma once

#include "Vector3.h"
#include "Quaternion.h"

namespace Math {

class Transformation
{
public:
    Vector3 position_;
    Vector3 scale_;
    Quaternion oriention_;

    Transformation() :
        position_(Vector3(0.0f, 0.0f, 0.0f)),
        scale_(Vector3(1.0f, 1.0f, 1.0f)),
        oriention_(Quaternion::Identity)
    {}
    Transformation(const Vector3& pos, const Vector3& scale, float rot) :
        position_(pos),
        scale_(scale),
        oriention_(Quaternion::FromAxisAngle(Vector3::UnitY, rot))
    {}
    Transformation(const Vector3& pos, const Vector3& scale, const Quaternion& rot) :
        position_(pos),
        scale_(scale),
        oriention_(rot)
    {}

    ~Transformation() = default;

    float GetYRotation() const;
    void SetYRotation(float rad);
    void LookAt(const Vector3& lookAt, const Vector3& up);

    /// Get transformation matrix
    XMath::XMMATRIX GetMatrix() const;
    /// Use rot instead of oriention_
    XMath::XMMATRIX GetMatrix(const Quaternion& rot) const;
};

}
