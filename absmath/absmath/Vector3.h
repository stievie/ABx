#pragma once

#include "MathDefs.h"
#include "MathUtils.h"
#include <sstream>
#include <cmath>
#include <array>

namespace Math {

class Vector4;

class Vector3
{
public:
    constexpr Vector3() noexcept :
        x_(0.0f),
        y_(0.0f),
        z_(0.0f)
    { }

    constexpr Vector3(const Vector3& vector) noexcept :
        x_(vector.x_),
        y_(vector.y_),
        z_(vector.z_)
    { }
    /// Create from Vector4, drop w_
    Vector3(const Vector4& vector) noexcept;

    constexpr Vector3(float x, float y, float z) noexcept :
        x_(x),
        y_(y),
        z_(z)
    { }
    constexpr Vector3(const std::array<float, 3>& values) noexcept :
        x_(values[0]),
        y_(values[1]),
        z_(values[2])
    { }

#if defined(HAVE_DIRECTX_MATH)
    Vector3(const XMath::XMVECTOR& vector) noexcept :
        x_(XMath::XMVectorGetX(vector)),
        y_(XMath::XMVectorGetY(vector)),
        z_(XMath::XMVectorGetZ(vector))
    { }
    Vector3(const XMath::XMFLOAT3& vector) noexcept :
        x_(vector.x),
        y_(vector.y),
        z_(vector.z)
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

    operator std::array<float, 3>() const
    {
        return { x_, y_, z_ };
    }
#if defined(HAVE_DIRECTX_MATH)
    /// Cast to XMVECTOR
    operator XMath::XMVECTOR() const
    {
        return XMath::XMVectorSet(x_, y_, z_, 0.0f);
    }
    operator XMath::XMFLOAT3() const
    {
        return{ x_, y_, z_ };
    }
#endif

    bool operator ==(const Vector3& vector) const { return Equals(vector); }
    bool operator !=(const Vector3& vector) const { return !Equals(vector); }

    /// Return negation.
    Vector3 operator -() const { return Vector3(-x_, -y_, -z_); }

    Vector3& operator+=(const Vector3& v);
    Vector3& operator-=(const Vector3& v);
    Vector3& operator*=(const Vector3& v);
    Vector3& operator/=(const Vector3& v);
    Vector3& operator*=(float v);
    Vector3& operator/=(float v);

    Vector3 operator+(const Vector3& v) const;
    Vector3 operator-(const Vector3& v) const;
    Vector3 operator+(float v) const;
    Vector3 operator-(float v) const;
    Vector3 operator*(const Vector3& v) const;
    Vector3 operator/(const Vector3& v) const;

    friend Vector3 operator*(const Vector3& v, float n);
    friend Vector3 operator*(float n, const Vector3& v);

    friend Vector3 operator/(const Vector3& v, float n);
    friend Vector3 operator/(float n, const Vector3& v);

    const Vector3 CrossProduct(const Vector3& v) const
    {
#if defined(HAVE_DIRECTX_MATH)
        return XMath::XMVector3Cross(*this, v);
#else
        return Vector3(
            y_ * v.z_ - z_ * v.y_,
            z_ * v.x_ - x_ * v.z_,
            x_ * v.y_ - y_ * v.x_
        );
#endif
    }

    float DotProduct(const Vector3& v) const
    {
        return x_ * v.x_ + y_ * v.y_ + z_ * v.z_;
    }

    float Angle(const Vector3& v) const
    {
        return acosf(DotProduct(v) / (Length() * v.Length()));
    }
    /// Returns the Y angle between this point and v in Rad
    float AngleY(const Vector3& v) const
    {
        return atan2f(x_ - v.x_, z_ - v.z_);
    }
    Vector3 Orthogonal() const;

    float LengthSqr() const;
    float Length() const;
    float Distance(const Vector3& v) const;
    const Vector3 Normal() const;
    void Normalize();
    /// Absolute
    Vector3 Abs() const
    {
        return Vector3(fabsf(x_), fabsf(y_), fabsf(z_));
    }
    /// Linear interpolation
    Vector3 Lerp(const Vector3& to, float i) const
    {
        return *this * (1.0f - i) + to * i;
    }
    /// Test for equality with another vector with epsilon.
    inline bool Equals(const Vector3& rhs) const
    {
        return Math::Equals(x_, rhs.x_) && Math::Equals(y_, rhs.y_) && Math::Equals(z_, rhs.z_);
    }
    void Clamp(const Vector3& min, const Vector3& max);
    void Clamp(float min, float max);

    std::string ToString() const
    {
        std::stringstream ss;
        ss << x_ << " " << y_ << " " << z_;
        return ss.str();
    }
    void SetLength(float length);

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

template<class _Stream>
inline _Stream& operator << (_Stream& os, const Vector3& value)
{
    return os << value.ToString();
}

}
