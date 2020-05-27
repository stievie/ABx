/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "Vector4.h"
#include "MathUtils.h"
#include <sa/StringTempl.h>

namespace Math {

const Vector4 Vector4::Zero;
const Vector4 Vector4::One(1.0f, 1.0f, 1.0f, 1.0f);
const Vector4 Vector4::UnitX(1.0f, 0.0f, 0.0f, 0.0f);
const Vector4 Vector4::UnitY(0.0f, 1.0f, 0.0f, 0.0f);
const Vector4 Vector4::UnitZ(0.0f, 0.0f, 1.0f, 0.0f);
const Vector4 Vector4::UnitW(0.0f, 0.0f, 0.0f, 1.0f);

Vector4::Vector4(const std::string& str)
{
    const std::vector<std::string> parts = sa::Split(str, " ");
    if (parts.size() > 2)
    {
        x_ = std::stof(parts.at(0));
        y_ = std::stof(parts.at(1));
        z_ = std::stof(parts.at(2));
        if (parts.size() > 3)
            w_ = std::stof(parts.at(3));
        else
            w_ = 1.0f;
    }
    else
    {
        x_ = 0.0f;
        y_ = 0.0f;
        z_ = 0.0f;
        w_ = 0.0f;
    }
}

Vector4& Vector4::operator+=(const Vector4& v)
{
    x_ += v.x_;
    y_ += v.y_;
    z_ += v.z_;
    w_ += v.w_;
    return *this;
}

Vector4& Vector4::operator-=(const Vector4& v)
{
    x_ -= v.x_;
    y_ -= v.y_;
    z_ -= v.z_;
    w_ -= v.w_;
    return *this;
}

const Vector4 Vector4::operator+(const Vector4& v) const
{
    return Vector4(x_ + v.x_, y_ + v.y_, z_ + v.z_, w_ + v.w_);
}

const Vector4 Vector4::operator-(const Vector4& v) const
{
    return Vector4(x_ - v.x_, y_ - v.y_, z_ - v.z_, w_ - v.w_);
}

Vector4 operator*(const Vector4& v, float n)
{
    return Vector4(v.x_ * n, v.y_ * n, v.z_ * n, v.w_ * n);
}

Vector4 operator*(float n, const Vector4& v)
{
    return Vector4(v.x_ * n, v.y_ * n, v.z_ * n, v.w_ * n);
}

Vector4 operator/(const Vector4& v, float n)
{
    return Vector4(v.x_ / n, v.y_ / n, v.z_ / n, v.w_ / n);
}

Vector4 operator/(float n, const Vector4& v)
{
    return Vector4(v.x_ / n, v.y_ / n, v.z_ / n, v.w_ / n);
}

float Vector4::LengthSqr() const
{
    return x_*x_ + y_*y_ + z_*z_ + w_*w_;
}

float Vector4::Length() const
{
    return sqrt(x_*x_ + y_*y_ + z_*z_ + w_*w_);
}

float Vector4::Distance(const Vector4& v) const
{
    return (*this - v).Length();
}

void Vector4::Normalize()
{
    float length = Length();
    x_ /= length;
    y_ /= length;
    z_ /= length;
    w_ /= length;
}

const Vector4 Vector4::Normal() const
{
    return *this / Length();
}

}
