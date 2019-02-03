#pragma once

#include "Vector3.h"

namespace Math {

class Line
{
public:
    Line() = default;
    inline Line(const Vector3& s, const Vector3& e) :
        start_(s),
        end_(e)
    { }
    ~Line() = default;

    Vector3 start_;
    Vector3 end_;
};

}
