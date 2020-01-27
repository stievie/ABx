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

#include <algorithm>

namespace Math {

template<typename T>
class Rect
{
public:
    constexpr Rect() noexcept {}
    constexpr Rect(T left, T top, T right, T bottom) noexcept :
        left_(left),
        top_(top),
        right_(right),
        bottom_(bottom)
    { }
    constexpr Rect(const Rect<T>& rect) noexcept :
        left_(rect.left_),
        top_(rect.top_),
        right_(rect.right_),
        bottom_(rect.bottom_)
    { }

    Rect<T>& operator =(const Rect<T>& rect)
    {
        left_ = rect.left_;
        top_ = rect.top_;
        right_ = rect.right_;
        bottom_ = rect.bottom_;
        return *this;
    }

    bool operator ==(const Rect<T>& rect) const
    {
        return left_ == rect.left_ && top_ == rect.top_ && right_ == rect.right_ && bottom_ == rect.bottom_;
    }
    bool operator !=(const Rect<T>& rect) const
    {
        return left_ != rect.left_ || top_ != rect.top_ || right_ != rect.right_ || bottom_ != rect.bottom_;
    }

    T Width() const
    {
        return right_ - left_;
    }
    T Height() const
    {
        return bottom_ - top_;
    }

    bool IsInside(T x, T y) const
    {
        return (left_ < x) && (right_ > x) &&
            (top_ < y) && (bottom_ > y);
    }
    /// Subtracts units from the left and top and adds units to the right and bottom.
    void Inflate(T x, T y)
    {
        left_ -= x;
        top_ -= y;
        right_ += x;
        bottom_ += y;
    }
    void Offset(T x, T y)
    {
        left_ += x;
        right_ += x;
        top_ += y;
        bottom_ += y;
    }
    void Normalize()
    {
        if (left_ > right_)
            std::swap(left_, right_);
        if (top_ > bottom_)
            std::swap(top_, bottom_);
    }

    T left_;
    T top_;
    T right_;
    T bottom_;
};

}
