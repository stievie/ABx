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

#include <array>

namespace Math {

static constexpr float M_EPSILON = 0.000001f;              // Small value
static constexpr float M_PIF = 3.14159265358979323846f;    // PI
static constexpr float M_TWOPI = M_PIF * 2.0f;             // 2*PI
static constexpr float M_PIHALF = M_PIF / 2.0f;            // PI/2
static constexpr float M_PIFOURTH = M_PIF / 4.0f;          // PI/4
static constexpr float M_INFINITE = std::numeric_limits<float>::infinity();

/// Intersection test result.
enum class Intersection
{
    Outside,
    Intersects,
    Inside
};

using StdVector3 = std::array<float, 3>;
using StdVector4 = std::array<float, 4>;

}
