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

#pragma once

#include "MathConfig.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4.h"

namespace Math {

class Transformation
{
public:
    Vector3 position_;
    Vector3 scale_;
    Quaternion oriention_;

    Transformation() :
        position_(Vector3::Zero),
        scale_(Vector3::One),
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
    void LookAt(const Vector3& lookAt, const Vector3& up = Math::Vector3::UnitY);

    /// Get transformation matrix
    Matrix4 GetMatrix() const;
    /// Use rot instead of oriention_
    Matrix4 GetMatrix(const Quaternion& rot) const;
    void Move(float speed, const Vector3& amount);
    void Turn(float yAngle);
    friend std::ostream& operator << (std::ostream& os, const Transformation& value)
    {
        return os << " position " << value.position_ << ", scale " << value.scale_ << ", orientation " << value.oriention_;
    }

};

}
