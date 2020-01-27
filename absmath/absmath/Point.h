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

#pragma once

namespace Math {

template<typename T>
class Point
{
public:
    constexpr Point() noexcept { }
    Point(T x, T y) :
        x_(x),
        y_(y)
    { }
    constexpr Point(const Point<T>& point) noexcept :
        x_(point.x_),
        y_(point.x_)
    { }

    Point<T>& operator =(const Point<T>& point)
    {
        x_ = point.x_;
        y_ = point.y_;
        return *this;
    }

    bool operator ==(const Point<T>& point) const
    {
        return x_ == point.x_ && y_ == point.y_;
    }
    bool operator !=(const Point<T>& point) const
    {
        return x_ != point.x_ || y_ != point.y_;
    }

    void Offset(T x, T y)
    {
        x_ += x;
        y_ += y;
    }

    T x_;
    T y_;
};

}
