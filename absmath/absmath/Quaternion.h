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

#include "Vector3.h"
#include "Vector4.h"
#include <string>

namespace Math {

class Matrix4;

class Quaternion
{
public:
    constexpr Quaternion() noexcept :
        w_(1.0f),
        x_(0.0f),
        y_(0.0f),
        z_(0.0f)
    {}
    constexpr Quaternion(float w, float x, float y, float z) noexcept :
        w_(w),
        x_(x),
        y_(y),
        z_(z)
    {}
    constexpr Quaternion(const std::array<float, 4>& values) noexcept :
        w_(values[0]),
        x_(values[1]),
        y_(values[2]),
        z_(values[3])
    {}
    Quaternion(const std::array<float, 3>& values) noexcept :
        // 0 = Pitch, 1 = Yaw, 2 = Roll
        Quaternion(values[0], values[1], values[2])
    {}
    Quaternion(float x, float y, float z);
#if defined(HAVE_DIRECTX_MATH)
    Quaternion(const XMath::XMVECTOR& q) :
        w_(XMath::XMVectorGetW(q)),
        x_(XMath::XMVectorGetX(q)),
        y_(XMath::XMVectorGetY(q)),
        z_(XMath::XMVectorGetZ(q))
    { }
#endif
    explicit Quaternion(const Vector3& eulerAngles) :
        // x = Pitch, y = Yaw, z = Roll
        Quaternion(eulerAngles.x_, eulerAngles.y_, eulerAngles.z_)
    {}
    /// Parse from string
    explicit Quaternion(const std::string& str);

    /// Create a Quaternion from Axis and Angle. Angle is Rad
    static Quaternion FromAxisAngle(const Vector3& axis, float angle);
    static Quaternion FromLookAt(const Vector3& from, const Vector3& to);

    operator std::array<float, 4>() const
    {
        return { x_, y_, z_, w_ };
    }
#if defined(HAVE_DIRECTX_MATH)
    /// Cast to XMVECTOR
    operator XMath::XMVECTOR() const
    {
        return XMath::XMVectorSet(x_, y_, z_, w_);
    }
    operator XMath::XMFLOAT4() const
    {
        return { x_, y_, z_, w_ };
    }
#endif

    /// Test for equality
    bool operator ==(const Quaternion& rhs) const {
        return Equals(rhs);
    }
    /// Test for inequality
    bool operator !=(const Quaternion& rhs) const {
        return !Equals(rhs);
    }

    Quaternion& operator+=(const Quaternion& v);
    Quaternion& operator-=(const Quaternion& v);

    const Quaternion operator+(const Quaternion& v) const;
    const Quaternion operator-(const Quaternion& v) const;

    friend Quaternion operator*(const Quaternion& v, float n);
    friend Quaternion operator*(float n, const Quaternion& v);
    /// Multiply Quaternions
    /// https://www.3dgep.com/understanding-quaternions/#Quaternion_Products
    friend Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs);
    Quaternion& operator*=(const Quaternion& rhs);

    friend Quaternion operator/(const Quaternion& v, float n);
    friend Quaternion operator/(float n, const Quaternion& v);

    Vector3 operator *(const Vector3& rhs) const
    {
        const Vector3 qVec(x_, y_, z_);
        const Vector3 cross1(qVec.CrossProduct(rhs));
        const Vector3 cross2(qVec.CrossProduct(cross1));

        return rhs + 2.0f * (cross1 * w_ + cross2);
    }
    /// Test for equality with another vector with epsilon.
    bool Equals(const Quaternion& rhs) const
    {
        return Math::Equals(x_, rhs.x_) && Math::Equals(y_, rhs.y_) && Math::Equals(z_, rhs.z_) && Math::Equals(w_, rhs.w_);
    }

    Quaternion Inverse() const;
    Quaternion Conjugate() const;

    /// Return Axis and Angle
    /// x, y, z -> Axis
    /// w -> Angle
    Vector4 AxisAngle() const;
    /// Convert to Euler angles
    /// x = roll
    /// y = pitch
    /// z = yaw
    Vector3 EulerAngles() const;
    const Quaternion Normal() const;
    void Normalize();
    float LengthSqr() const;
    float Length() const;
    /// Return rotation matrix
    Matrix4 GetMatrix() const;

    std::string ToString() const
    {
        std::stringstream ss;
        ss << w_ << " " << x_ << " " << y_ << " " << z_;
        return ss.str();
    }

    float w_;
    float x_;
    float y_;
    float z_;

    static const Quaternion Identity;
};

template<class _Stream>
inline _Stream& operator << (_Stream& os, const Quaternion& value)
{
    return os << value.ToString();
}

}
