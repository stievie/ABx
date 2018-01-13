#pragma once

namespace Math {

template<typename T>
class Rect
{
public:
    Rect() {};
    Rect(T left, T top, T right, T bottom) :
        left_(left),
        top_(top),
        right_(right),
        bottom_(bottom)
    { }
    Rect(const Rect<T>& rect) :
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
        right += x;
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
