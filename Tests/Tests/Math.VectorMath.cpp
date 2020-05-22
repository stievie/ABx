/**
 * Copyright 2020 Stefan Ascher
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


#include <catch.hpp>

#include <absmath/VectorMath.h>

TEST_CASE("IsPointInTriangle")
{
    Math::Vector3 p1{ 0.0f, 0.0f, -1.0f };
    Math::Vector3 p2{ 1.0f, 0.0f, 0.0f };
    Math::Vector3 p3{ 0.0f, 0.0f, 1.0f };

    Math::Vector3 pt1{ 0.0f, 0.0f, 0.0f };
    bool res = Math::IsPointInTriangle(pt1, p1, p2, p3);
    REQUIRE(res);

    Math::Vector3 pt2{ 5.0f, 0.0f, 0.0f };
    bool res2 = Math::IsPointInTriangle(pt2, p1, p2, p3);
    REQUIRE(!res2);
}
