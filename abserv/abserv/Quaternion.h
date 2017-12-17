#pragma once

#include "Vector3.h"
#include "Vector4.h"

namespace Math {

class Quaternion
{
public:
    Quaternion() :
        x_(0.0f),
        y_(0.0f),
        z_(0.0f),
        w_(0.0f)
    {}
    Quaternion(const Vector3& v, float w) :
        x_(v.x_),
        y_(v.y_),
        z_(v.z_),
        w_(w)
    {}
    Quaternion(float x, float y, float z, float w) :
        x_(x),
        y_(y),
        z_(z),
        w_(w)
    {}
    Quaternion(float pitch, float yaw, float roll);
#ifdef HAVE_DIRECTX_MATH
    Quaternion(const DirectX::XMVECTOR& q) :
        x_(q.m128_f32[0]),
        y_(q.m128_f32[1]),
        z_(q.m128_f32[2]),
        w_(q.m128_f32[3])
    { }
#endif
    explicit Quaternion(const Vector3& eulerAngles) :
        Quaternion(eulerAngles.x_, eulerAngles.y_, eulerAngles.z_)
    {}
    /// Parse from string
    explicit Quaternion(const std::string& str);

    /// Create a Quaternion from Axis and Angle
    static Quaternion FromAxisAngle(const Vector3& axis, float angle);

#ifdef HAVE_DIRECTX_MATH
    /// Cast to XMVECTOR
    operator DirectX::XMVECTOR() const
    {
        return DirectX::XMVectorSet(x_, y_, z_, w_);
    }
#endif

    /// Test for equality
    bool operator ==(const Quaternion& vector) const {
        return x_ == vector.x_ && y_ == vector.y_ && z_ == vector.z_ && w_ == vector.w_;
    }
    /// Test for inequality
    bool operator !=(const Quaternion& vector) const {
        return x_ != vector.x_ || y_ != vector.y_ || z_ != vector.z_ || w_ != vector.w_;
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

    float x_;
    float y_;
    float z_;
    float w_;

    static const Quaternion Identity;
};

}
