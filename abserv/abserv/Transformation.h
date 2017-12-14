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
    Quaternion rotation_;

    Transformation() :
        position_(Vector3(0.0f, 0.0f, 0.0f)),
        scale_(Vector3(1.0f, 1.0f, 1.0f)),
        rotation_(Quaternion::Identity)
    {}
    ~Transformation() {};

    void LookAt(const Vector3& position, const Vector3& up);
    void LookAt(const Vector3& position);

    DirectX::XMMATRIX GetMatrix() const;
};

}
