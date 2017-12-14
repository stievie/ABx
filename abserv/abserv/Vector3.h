#pragma once

namespace Math {

class Vector3
{
public:
    Vector3() :
        x_(0.0f),
        y_(0.0f),
        z_(0.0f)
    { }

    Vector3(const Vector3& vector) :
        x_(vector.x_),
        y_(vector.y_),
        z_(vector.z_)
    { }

    Vector3(float x, float y, float z) :
        x_(x),
        y_(y),
        z_(z)
    { }

#ifdef HAVE_DIRECTX_MATH
    Vector3(const DirectX::XMVECTOR& vector) :
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

#ifdef HAVE_DIRECTX_MATH
    /// Cast to XMVECTOR
    operator DirectX::XMVECTOR() const
    {
        return DirectX::XMVectorSet(x_, y_, z_, 0.0f);
    }
#endif

    bool operator ==(const Vector3& vector) const { return x_ == vector.x_ && y_ == vector.y_ && z_ == vector.z_; }
    bool operator !=(const Vector3& vector) const { return x_ != vector.x_ || y_ != vector.y_ || z_ != vector.z_; }

    Vector3& operator+=(const Vector3& v);
    Vector3& operator-=(const Vector3& v);

    Vector3 operator+(const Vector3& v) const;
    Vector3 operator-(const Vector3& v) const;

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
        return acos(DotProduct(v) / Length() / v.Length());
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

}
