#include "stdafx.h"
#include "Vector3.h"
#include "Utils.h"
#include "Logger.h"

#include "DebugNew.h"

namespace Math {

const Vector3 Vector3::Zero;
const Vector3 Vector3::One(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::UnitX(1.0f, 0.0f, 0.0f);           // Right
const Vector3 Vector3::UnitY(0.0f, 1.0f, 0.0f);           // Up
const Vector3 Vector3::UnitZ(0.0f, 0.0f, 1.0f);           // Forward
const Vector3 Vector3::Back(0.0f, 0.0f, -1.0f);
const Vector3 Vector3::Down(0.0f, -1.0f, 0.0f);
const Vector3 Vector3::Left(-1.0f, 0.0f, 0.0f);

Vector3::Vector3(const std::string& str)
{
    std::vector<std::string> parts = Utils::Split(str, " ");
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
#ifdef DEBUG_MATH
        LOG_ERROR << "Malformed Vector3 string: " << str << std::endl;
#endif
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
    return *this / Length();
}

}
