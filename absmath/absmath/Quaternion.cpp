#include "stdafx.h"
#include "Quaternion.h"
#include "MathUtils.h"

namespace Math {

const Quaternion Quaternion::Identity(1.0f, 0.0f, 0.0f, 0.0f);

Quaternion::Quaternion(float x, float y, float z)
{
    x *= 0.5f;
    y *= 0.5f;
    z *= 0.5f;

    float sinX = sinf(x);
    float cosX = cosf(x);
    float sinY = sinf(y);
    float cosY = cosf(y);
    float sinZ = sinf(z);
    float cosZ = cosf(z);

    w_ = cosY * cosX * cosZ + sinY * sinX * sinZ;
    x_ = cosY * sinX * cosZ + sinY * cosX * sinZ;
    y_ = sinY * cosX * cosZ - cosY * sinX * sinZ;
    z_ = cosY * cosX * sinZ - sinY * sinX * cosZ;
}

Quaternion::Quaternion(const std::string& str)
{
    std::vector<std::string> parts = Math::Split(str, ' ');

    if (parts.size() < 3)
    {
        w_ = 1.0f;
        x_ = 0.0f;
        y_ = 0.0f;
        z_ = 0.0f;
        return;
    }
    if (parts.size() < 4)
    {
        // Euler angles
        *this = Quaternion(
            std::stof(parts.at(1)),   // x
            std::stof(parts.at(2)),   // y
            std::stof(parts.at(3)));  // z
        return;
    }

    w_ = std::stof(parts.at(0));
    x_ = std::stof(parts.at(1));
    y_ = std::stof(parts.at(2));
    z_ = std::stof(parts.at(3));
}

Quaternion Quaternion::FromTwoVectors(const Vector3& u, const Vector3& v)
{
    // http://lolengine.net/blog/2014/02/24/quaternion-from-two-vectors-final
    float normUNormV = sqrt(u.DotProduct(u) * v.DotProduct(v));
    float realPart = normUNormV + u.DotProduct(v);
    Vector3 w;
    if (realPart < 1.e-6f * normUNormV)
    {
        /* If u and v are exactly opposite, rotate 180 degrees
        * around an arbitrary orthogonal axis. Axis normalisation
        * can happen later, when we normalise the quaternion. */
        realPart = 0.0f;
        w = abs(u.x_) > abs(u.z_)
            ? Vector3(-u.y_, u.x_, 0.0f)
            : Vector3(0.0f, -u.z_, u.y_);
    }
    else
    {
        w = u.CrossProduct(v);
    }
    return Quaternion::FromAxisAngle(w, realPart);
}

Quaternion Quaternion::FromAxisAngle(const Vector3& axis, float angle)
{
    const Math::Vector3 normalAxis = axis.Normal();
    float factor = sin(angle * 0.5f);
    float x = normalAxis.x_ * factor;
    float y = normalAxis.y_ * factor;
    float z = normalAxis.z_ * factor;

    float w = cos(angle / 2.0f);
    return Quaternion(w, x, y, z);
}

Vector4 Quaternion::AxisAngle() const
{
    Quaternion q(w_, x_, y_, z_);
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
    // Derivation from http://www.geometrictools.com/Documentation/EulerAngles.pdf
    // Order of rotations: Z first, then X, then Y
    float check = 2.0f * (-y_ * z_ + w_ * x_);

    if (check < -0.995f)
    {
        return Vector3(
            -float(M_PI_2),
            0.0f,
            -atan2f(2.0f * (x_ * z_ - w_ * y_), 1.0f - 2.0f * (y_ * y_ + z_ * z_))
        );
    }
    else if (check > 0.995f)
    {
        return Vector3(
            float(M_PI_2),
            0.0f,
            atan2f(2.0f * (x_ * z_ - w_ * y_), 1.0f - 2.0f * (y_ * y_ + z_ * z_))
        );
    }
    else
    {
        return Vector3(
            asinf(check),
            atan2f(2.0f * (x_ * z_ + w_ * y_), 1.0f - 2.0f * (x_ * x_ + y_ * y_)),
            atan2f(2.0f * (x_ * y_ + w_ * z_), 1.0f - 2.0f * (x_ * x_ + z_ * z_))
        );
    }
}

Quaternion Quaternion::Inverse() const
{
#if defined(HAVE_DIRECTX_MATH) || defined(HAVE_X_MATH)
    return XMath::XMQuaternionInverse(*this);
#else
    float lenSquared = LengthSqr();
    if (lenSquared == 1.0f)
        return Conjugate();
    else if (lenSquared >= M_EPSILON)
        return Conjugate() * (1.0f / lenSquared);
    else
        return Identity;
#endif
}
Quaternion Quaternion::Conjugate() const
{
#if defined(HAVE_DIRECTX_MATH) || defined(HAVE_X_MATH)
    return XMath::XMQuaternionConjugate(*this);
#else
    return Quaternion(-x_, -y_, -z_, w_);
#endif
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
    const float length = Length();
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
