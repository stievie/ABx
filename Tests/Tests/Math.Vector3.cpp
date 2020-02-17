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

#include "stdafx.h"
#include <catch.hpp>

#include <absmath/Vector3.h>
#include <absmath/Vector4.h>

TEST_CASE("Vector3 Construct", "[vector3]")
{
    SECTION("Parse from string")
    {
        Math::Vector3 vec("1.0 2.0 1.0");
        REQUIRE(vec.x_ == 1.0f);
        REQUIRE(vec.y_ == 2.0f);
        REQUIRE(vec.z_ == 1.0f);
    }
}

TEST_CASE("Vector3 Compare", "[vector3]")
{
    SECTION("Compare equality")
    {
        Math::Vector3 vec1(1.0f, 2.0f, 1.0f);
        Math::Vector3 vec2(1.0f, 2.0f, 1.0f);
        REQUIRE(vec1 == vec2);
    }
    SECTION("Compare inequality")
    {
        Math::Vector3 vec1(1.0f, 2.0f, 1.0f);
        Math::Vector3 vec2(1.0f, 1.0f, 1.0f);
        REQUIRE(vec1 != vec2);
    }
}

TEST_CASE("Vector3 Arithmetic", "[vector3]")
{
    SECTION("Negate")
    {
        Math::Vector3 vec1(1.0f, 2.0f, 1.0f);
        Math::Vector3 vec2 = -vec1;
        REQUIRE(vec2 == Math::Vector3(-1.0f, -2.0f, -1.0f));
    }
    SECTION("Increment +=")
    {
        Math::Vector3 vec1(1.0f, 2.0f, 1.0f);
        Math::Vector3 vec2(1.0f, 1.0f, 1.0f);
        vec2 += vec1;
        REQUIRE(vec2 == Math::Vector3(2.0f, 3.0f, 2.0f));
    }
}

TEST_CASE("Vector3 Geometry", "[vector3]")
{
    SECTION("CrossProduct")
    {
        Math::Vector3 vec1(0.0f, 0.0f, 1.0f);
        Math::Vector3 vec2(1.0f, 0.0f, 0.0f);
        Math::Vector3 cross = vec1.CrossProduct(vec2);
        REQUIRE(cross.Equals(Math::Vector3(0.0f, 1.0f, 0.0f)));

        // Calculated by "hand"
        Math::Vector3 cp(
            vec1.y_ * vec2.z_ - vec1.z_ * vec2.y_,
            vec1.z_ * vec2.x_ - vec1.x_ * vec2.z_,
            vec1.x_ * vec2.y_ - vec1.y_ * vec2.x_
        );
        REQUIRE(cross.Equals(cp));
    }

}
