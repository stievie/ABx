#pragma once

#include "Vector3.h"
#include "Vector4.h"
#include <string>

namespace Math {

class Quaternion
{
public:
    Quaternion() noexcept :
        x_(0.0f),
        y_(0.0f),
        z_(0.0f),
        w_(0.0f)
    {}
    Quaternion(const Vector3& v, float w) noexcept :
        x_(v.x_),
        y_(v.y_),
        z_(v.z_),
        w_(w)
    {}
    Quaternion(float x, float y, float z, float w) noexcept :
        x_(x),
        y_(y),
        z_(z),
        w_(w)
    {}
    Quaternion(float pitch, float yaw, float roll);
#if defined(HAVE_DIRECTX_MATH) || defined(HAVE_X_MATH)
    Quaternion(const XMath::XMVECTOR& q) :
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

    /// Create a Quaternion representing the rotation between two 3D vectors
    static Quaternion FromTwoVectors(const Vector3& u, const Vector3& v);
    /// Create a Quaternion from Axis and Angle. Angle is Rad
    static Quaternion FromAxisAngle(const Vector3& axis, float angle);

#if defined(HAVE_DIRECTX_MATH) || defined(HAVE_X_MATH)
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

    Vector3 operator *(const Vector3& rhs) const
    {
        Vector3 qVec(x_, y_, z_);
        Vector3 cross1(qVec.CrossProduct(rhs));
        Vector3 cross2(qVec.CrossProduct(cross1));

        return rhs + 2.0f * (cross1 * w_ + cross2);
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

    std::string ToString() const
    {
        std::stringstream ss;
        ss << x_ << " " << y_ << " " << z_ << " " << w_;
        return ss.str();
    }

    float x_;
    float y_;
    float z_;
    float w_;

    static const Quaternion Identity;
};

}
