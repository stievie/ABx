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

#include <absmath/MathUtils.h>

TEST_CASE("NormalizeAngle")
{
    SECTION("Negative")
    {
        float angle = Math::DegToRad(-90.0f);
        float nomalAngle = angle;
        Math::NormalizeAngle(nomalAngle);
        nomalAngle = Math::RadToDeg(nomalAngle);
        REQUIRE(nomalAngle == Approx(270.0f));
    }
    SECTION("Greater 360")
    {
        float angle = Math::DegToRad(390.0f);
        float nomalAngle = angle;
        Math::NormalizeAngle(nomalAngle);
        nomalAngle = Math::RadToDeg(nomalAngle);
        REQUIRE(nomalAngle == Approx(30.0f));

        angle = Math::DegToRad(390.0f) + Math::M_TWOPI;
        nomalAngle = angle;
        Math::NormalizeAngle(nomalAngle);
        nomalAngle = Math::RadToDeg(nomalAngle);
        REQUIRE(nomalAngle == Approx(30.0f));
    }
}

TEST_CASE("Lerp")
{
    float r = Math::Lerp(1.0f, 2.0f, 0.5f);
    REQUIRE(r == Approx(1.5f));
}
