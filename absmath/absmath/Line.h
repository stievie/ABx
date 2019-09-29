#pragma once

#include "Vector3.h"

namespace Math {

class Line
{
public:
    constexpr Line() noexcept = default;
    constexpr  Line(const Vector3& s, const Vector3& e) noexcept :
        start_(s),
        end_(e)
    { }
    ~Line() = default;

    Vector3 start_;
    Vector3 end_;
};

}
