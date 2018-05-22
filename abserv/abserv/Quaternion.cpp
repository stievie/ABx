#include "stdafx.h"
#include "Quaternion.h"
#include "Logger.h"
#include "StringUtils.h"

#include "DebugNew.h"

namespace Math {

const Quaternion Quaternion::Identity(0.0f, 0.0f, 0.0f, 1.0f);

Quaternion::Quaternion(float pitch, float yaw, float roll)
{
    yaw *= 0.5f;
    pitch *= 0.5f;
    roll *= 0.5f;

    float c1 = cos(yaw);
    float c2 = cos(pitch);
    float c3 = cos(roll);
    float s1 = sin(yaw);
    float s2 = sin(pitch);
    float s3 = sin(roll);

    w_ = c1 * c2 * c3 - s1 * s2 * s3;
    x_ = s1 * s2 * c3 + c1 * c2 * s3;
    y_ = s1 * c2 * c3 + c1 * s2 * s3;
    z_ = c1 * s2 * c3 - s1 * c2 * s3;
}

Quaternion::Quaternion(const std::string& str)
{
    std::vector<std::string> parts = Utils::Split(str, " ");

    if (parts.size() > 2)
    {
        x_ = std::stof(parts.at(0));
        y_ = std::stof(parts.at(1));
        z_ = std::stof(parts.at(2));
        if (parts.size() > 3)
            w_ = std::stof(parts.at(3));
        else
        {
            // Euler angles
            *this = Quaternion(x_, y_, z_);
        }
    }
    else
    {
        x_ = 0.0f;
        y_ = 0.0f;
        z_ = 0.0f;
        w_ = 0.0f;
#ifdef DEBUG_MATH
        LOG_ERROR << "Malformed Quaternion string: " << str << std::endl;
#endif
    }
}

Quaternion Quaternion::FromAxisAngle(const Vector3& axis, float angle)
{
    float factor = sin(angle / 2.0f);
    float x = axis.x_ * factor;
    float y = axis.y_ * factor;
    float z = axis.z_ * factor;

    float w = cos(angle / 2.0f);
    return Quaternion(x, y, z, w).Normal();
}

Vector4 Quaternion::AxisAngle() const
{
    Quaternion q(x_, y_, z_, w_);
    if (abs(q.w_) > 1.0f)
        q.Normalize();

    float den = sqrt(1.0f - q.w_ * q.w_);
    if (den > 0.0001f)
    {
        return Vector4(
            q.x_ / den,
            q.y_ / den,
            q.z_ / den,
            2.0f * acos(q.w_)   // Angle
        );
    }
    else
    {
        return Vector4(
            Vector3::UnitX.x_,
            Vector3::UnitY.y_,
            Vector3::UnitZ.z_,
            2.0f * acos(q.w_)   // Angle
        );
    }
}

Vector3 Quaternion::EulerAngles() const
{
    float ysqr = y_ * y_;

    // roll (x-axis rotation)
    float t0 = +2.0f * (w_ * x_ + y_ * z_);
    float t1 = +1.0f - 2.0f * (x_ * x_ + ysqr);
    float roll = std::atan2(t0, t1);

    // pitch (y-axis rotation)
    float t2 = +2.0f * (w_ * y_ - z_ * x_);
    t2 = t2 > 1.0f ? 1.0f : t2;
    t2 = t2 < -1.0f ? -1.0f : t2;
    float pitch = std::asin(t2);

    // yaw (z-axis rotation)
    float t3 = +2.0f * (w_ * z_ + x_ * y_);
    float t4 = +1.0f - 2.0f * (ysqr + z_ * z_);
    float yaw = std::atan2(t3, t4);

    return Vector3(roll, pitch, yaw);
}

Quaternion& Quaternion::operator+=(const Quaternion& v)
{
    x_ += v.x_;
    y_ += v.y_;
    z_ += v.z_;
    w_ += v.w_;
    return *this;
}

Quaternion& Quaternion::operator-=(const Quaternion& v)
{
    x_ -= v.x_;
    y_ -= v.y_;
    z_ -= v.z_;
    w_ -= v.w_;
    return *this;
}

Quaternion& Quaternion::operator*=(const Quaternion& rhs)
{
    x_ = x_ * rhs.w_ + y_ * rhs.z_ - z_ * rhs.y_ + w_ * rhs.x_;
    y_ = -x_ * rhs.z_ + y_ * rhs.w_ + z_ * rhs.x_ + w_ * rhs.y_;
    z_ = x_ * rhs.y_ - y_ * rhs.x_ + z_ * rhs.w_ + w_ * rhs.z_;
    w_ = -x_ * rhs.x_ - y_ * rhs.y_ - z_ * rhs.z_ + w_ * rhs.w_;
    return *this;
}

const Quaternion Quaternion::operator+(const Quaternion& v) const
{
    return Quaternion(x_ + v.x_, y_ + v.y_, z_ + v.z_, w_ + v.w_);
}

const Quaternion Quaternion::operator-(const Quaternion& v) const
{
    return Quaternion(x_ - v.x_, y_ - v.y_, z_ - v.z_, w_ - v.w_);
}

Quaternion operator*(const Quaternion& v, float n)
{
    return Quaternion(v.x_ * n, v.y_ * n, v.z_ * n, v.w_ * n);
}

Quaternion operator*(float n, const Quaternion& v)
{
    return Quaternion(v.x_ * n, v.y_ * n, v.z_ * n, v.w_ * n);
}

Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs)
{
    return Quaternion(
        lhs.x_ * rhs.w_ + lhs.y_ * rhs.z_ - lhs.z_ * rhs.y_ + lhs.w_ * rhs.x_,
        -lhs.x_ * rhs.z_ + lhs.y_ * rhs.w_ + lhs.z_ * rhs.x_ + lhs.w_ * rhs.y_,
        lhs.x_ * rhs.y_ - lhs.y_ * rhs.x_ + lhs.z_ * rhs.w_ + lhs.w_ * rhs.z_,
        -lhs.x_ * rhs.x_ - lhs.y_ * rhs.y_ - lhs.z_ * rhs.z_ + lhs.w_ * rhs.w_
    );
}

Quaternion operator/(const Quaternion& v, float n)
{
    return Quaternion(v.x_ / n, v.y_ / n, v.z_ / n, v.w_ / n);
}

Quaternion operator/(float n, const Quaternion& v)
{
    return Quaternion(v.x_ / n, v.y_ / n, v.z_ / n, v.w_ / n);
}

void Quaternion::Normalize()
{
    float length = Length();
    x_ /= length;
    y_ /= length;
    z_ /= length;
    w_ /= length;
}

const Quaternion Quaternion::Normal() const
{
    return *this / Length();
}

float Quaternion::Length() const
{
    return sqrt(w_*w_ + x_*x_ + y_*y_ + z_*z_);
}

float Quaternion::LengthSqr() const
{
    return (w_*w_ + x_*x_ + y_*y_ + z_*z_);
}

}
