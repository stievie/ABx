#pragma once

#include "MathDefs.h"
#include "MathUtils.h"
#include <sstream>

namespace Math {

class Vector3
{
public:
    Vector3() noexcept :
        x_(0.0f),
        y_(0.0f),
        z_(0.0f)
    { }

    Vector3(const Vector3& vector) noexcept :
        x_(vector.x_),
        y_(vector.y_),
        z_(vector.z_)
    { }

    Vector3(float x, float y, float z) noexcept :
        x_(x),
        y_(y),
        z_(z)
    { }

#if defined(HAVE_DIRECTX_MATH) || defined(HAVE_X_MATH)
        Vector3(const XMath::XMVECTOR& vector) noexcept :
        x_(vector.m128_f32[0]),
        y_(vector.m128_f32[1]),
        z_(vector.m128_f32[2])
    { }
#endif

    /// Parse from string
    explicit Vector3(const std::string& str);

    Vector3& operator =(const Vector3& vector)
    {
        x_ = vector.x_;
        y_ = vector.y_;
        z_ = vector.z_;
        return *this;
    }

#if defined(HAVE_DIRECTX_MATH) || defined(HAVE_X_MATH)
    /// Cast to XMVECTOR
    operator XMath::XMVECTOR() const
    {
        return XMath::XMVectorSet(x_, y_, z_, 0.0f);
    }
#endif

    bool operator ==(const Vector3& vector) const { return x_ == vector.x_ && y_ == vector.y_ && z_ == vector.z_; }
    bool operator !=(const Vector3& vector) const { return x_ != vector.x_ || y_ != vector.y_ || z_ != vector.z_; }

    /// Return negation.
    Vector3 operator -() const { return Vector3(-x_, -y_, -z_); }

    Vector3& operator+=(const Vector3& v);
    Vector3& operator-=(const Vector3& v);

    Vector3 operator+(const Vector3& v) const;
    Vector3 operator-(const Vector3& v) const;
    Vector3 operator+(float v) const;
    Vector3 operator-(float v) const;

    friend Vector3 operator*(const Vector3& v, float n);
    friend Vector3 operator*(float n, const Vector3& v);

    friend Vector3 operator/(const Vector3& v, float n);
    friend Vector3 operator/(float n, const Vector3& v);

    const Vector3 CrossProduct(const Vector3& v) const
    {
        return Vector3(
            y_ * v.z_ - z_ * v.y_,
            z_ * v.x_ - x_ * v.z_,
            x_ * v.y_ - y_ * v.x_
        );
    }

    float DotProduct(const Vector3& v) const
    {
        return x_ * v.x_ + y_ * v.y_ + z_ * v.z_;
    }

    float Angle(const Vector3& v) const
    {
        return acos(DotProduct(v) / (Length() * v.Length()));
    }
    float AngleY(const Vector3& v) const
    {
        return atan2((x_ - v.x_), (z_ - v.z_)) * 180.0f / Math::M_PIF;
    }

    float LengthSqr() const;
    float Length() const;
    float Distance(const Vector3& v) const;
    const Vector3 Normal() const;
    /// Absolute
    Vector3 Abs() const
    {
        return Vector3(fabs(x_), fabs(y_), fabs(z_));
    }
    /// Linear interpolation
    Vector3 Lerp(const Vector3& to, float i) const
    {
        return *this * (1.0f - i) + to * i;
    }
    /// Test for equality with another vector with epsilon.
    bool Equals(const Vector3& rhs) const
    {
        return Math::Equals(x_, rhs.x_) && Math::Equals(y_, rhs.y_) && Math::Equals(z_, rhs.z_);
    }

    std::string ToString() const
    {
        std::stringstream ss;
        ss << x_ << " " << y_ << " " << z_;
        return ss.str();
    }

    /// Return integer data.
    const float* Data() const { return &x_; }

    float x_;
    float y_;
    float z_;

    static const Vector3 Zero;
    static const Vector3 One;
    static const Vector3 UnitX;
    static const Vector3 UnitY;
    static const Vector3 UnitZ;
    static const Vector3 Back;
    static const Vector3 Down;
    static const Vector3 Left;
};

inline Vector3 operator *(const Vector3& lhs, const Vector3& rhs)
{
    return Vector3(lhs.x_ * rhs.x_, lhs.y_ * rhs.y_, lhs.z_ * rhs.z_);
}

}
