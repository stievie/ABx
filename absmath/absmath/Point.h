#pragma once

namespace Math {

template<typename T>
class Point
{
public:
    Point() {};
    Point(T x, T y) :
        x_(x),
        y_(y)
    { }
    Point(const Point<T>& point) :
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
