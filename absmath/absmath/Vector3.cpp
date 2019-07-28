#include "stdafx.h"
#include "Vector3.h"
#include "MathUtils.h"
#include "Vector4.h"
#include "MathUtils.h"

namespace Math {

const Vector3 Vector3::Zero;
const Vector3 Vector3::One(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::UnitX(1.0f, 0.0f, 0.0f);           // Right
const Vector3 Vector3::UnitY(0.0f, 1.0f, 0.0f);           // Up
const Vector3 Vector3::UnitZ(0.0f, 0.0f, 1.0f);           // Forward
const Vector3 Vector3::Back(0.0f, 0.0f, -1.0f);
const Vector3 Vector3::Down(0.0f, -1.0f, 0.0f);
const Vector3 Vector3::Left(-1.0f, 0.0f, 0.0f);

Vector3::Vector3(const Vector4& vector) noexcept :
    x_(vector.x_),
    y_(vector.y_),
    z_(vector.z_)
{ }

Vector3::Vector3(const std::string& str)
{
    std::vector<std::string> parts = Math::Split(str, ' ');
    if (parts.size() == 3)
    {
        x_ = std::stof(parts.at(0));
        y_ = std::stof(parts.at(1));
        z_ = std::stof(parts.at(2));
    }
    else
    {
        x_ = 0.0f;
        y_ = 0.0f;
        z_ = 0.0f;
    }
}

Vector3& Vector3::operator+=(const Vector3& v)
{
    x_ += v.x_;
    y_ += v.y_;
    z_ += v.z_;
    return *this;
}

Vector3& Vector3::operator-=(const Vector3& v)
{
    x_ -= v.x_;
    y_ -= v.y_;
    z_ -= v.z_;
    return *this;
}

Vector3 Vector3::operator+(const Vector3& v) const
{
    return Vector3(x_ + v.x_, y_ + v.y_, z_ + v.z_);
}

Vector3 Vector3::operator-(const Vector3& v) const
{
    return Vector3(x_ - v.x_, y_ - v.y_, z_ - v.z_);
}

Vector3 Vector3::operator+(float v) const
{
    return Vector3(x_ + v, y_ + v, z_ + v);
}

Vector3 Vector3::operator-(float v) const
{
    return Vector3(x_ - v, y_ - v, z_ - v);
}

Vector3 operator*(const Vector3& v, float n)
{
    return Vector3(v.x_ * n, v.y_ * n, v.z_ * n);
}

Vector3 operator*(float n, const Vector3& v)
{
    return Vector3(v.x_ * n, v.y_ * n, v.z_ * n);
}

Vector3 operator/(const Vector3& v, float n)
{
    return Vector3(v.x_ / n, v.y_ / n, v.z_ / n);
}

Vector3 operator/(float n, const Vector3& v)
{
    return Vector3(v.x_ / n, v.y_ / n, v.z_ / n);
}

Vector3 Vector3::Orthogonal() const
{
    float x = fabs(x_);
    float y = fabs(y_);
    float z = fabs(z_);
    Vector3 other = (x < y) ?
        ((x < z) ? Vector3::UnitX : Vector3::UnitZ) :
        ((y < z) ? Vector3::UnitY : Vector3::UnitZ);
    return CrossProduct(other);
}

float Vector3::LengthSqr() const
{
    return x_*x_ + y_*y_ + z_*z_;
}

float Vector3::Length() const
{
    return sqrt(x_*x_ + y_*y_ + z_*z_);
}

float Vector3::Distance(const Vector3& v) const
{
    return (*this - v).Length();
}

const Vector3 Vector3::Normal() const
{
    float l = Length();
    if (!Math::Equals(l, 0.0f))
        return *this / l;
    return *this;
}

}
