#pragma once

#include "Vector3.h"

namespace Math {

class Vector4
{
public:
    Vector4() noexcept :
        x_(0.0f),
        y_(0.0f),
        z_(0.0f),
        w_(0.0f)
    { }

    Vector4(const Vector4& vector) noexcept :
        x_(vector.x_),
        y_(vector.y_),
        z_(vector.z_),
        w_(vector.w_)
    { }

    Vector4(float x, float y, float z, float w) noexcept :
        x_(x),
        y_(y),
        z_(z),
        w_(w)
    { }

    Vector4(const Vector3& vector, float w = 1.0f) noexcept :
        x_(vector.x_),
        y_(vector.y_),
        z_(vector.z_),
        w_(w)
    { }

#ifdef HAVE_DIRECTX_MATH
    Vector4(const DirectX::XMVECTOR& vector) noexcept :
        x_(vector.m128_f32[0]),
        y_(vector.m128_f32[1]),
        z_(vector.m128_f32[2]),
        w_(vector.m128_f32[3])
    { }
#endif

    /// Parse from string
    explicit Vector4(const std::string& str);

    Vector4& operator =(const Vector4& vector)
    {
        x_ = vector.x_;
        y_ = vector.y_;
        z_ = vector.z_;
        w_ = vector.w_;
        return *this;
    }

#ifdef HAVE_DIRECTX_MATH
    /// Cast to XMVECTOR
    operator DirectX::XMVECTOR() const
    {
        return DirectX::XMVectorSet(x_, y_, z_, w_);
    }
#endif

    bool operator ==(const Vector4& vector) const { return x_ == vector.x_ && y_ == vector.y_ && z_ == vector.z_ && w_ == vector.w_; }
    bool operator !=(const Vector4& vector) const { return x_ != vector.x_ || y_ != vector.y_ || z_ != vector.z_ || w_ != vector.w_; }

    Vector4& operator+=(const Vector4& v);
    Vector4& operator-=(const Vector4& v);

    const Vector4 operator+(const Vector4& v) const;
    const Vector4 operator-(const Vector4& v) const;

    friend Vector4 operator*(const Vector4& v, float n);
    friend Vector4 operator*(float n, const Vector4& v);

    friend Vector4 operator/(const Vector4& v, float n);
    friend Vector4 operator/(float n, const Vector4& v);

    float LengthSqr() const;
    float Length() const;
    float Distance(const Vector4& v) const;
    const Vector4 Normal() const;
    void Normalize();

    float x_;
    float y_;
    float z_;
    float w_;

    static const Vector4 Zero;
    static const Vector4 One;
    static const Vector4 UnitX;
    static const Vector4 UnitY;
    static const Vector4 UnitZ;
    static const Vector4 UnitW;
};

}
