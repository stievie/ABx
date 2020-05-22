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

#include <absmath/Shape.h>
#include <absmath/BoundingBox.h>
#include <absmath/Sphere.h>

TEST_CASE("IsTriangles")
{
    Math::BoundingBox bb(-2.0f, 2.0f);
    Math::Shape bbShape = bb.GetShape();
    REQUIRE(bbShape.IsTriangles());
}

TEST_CASE("BB IsFacingOutside")
{
    Math::BoundingBox bb(0.0f, 1.0f);
    Math::Shape bbShape = bb.GetShape();
    for (unsigned i = 0; i < bbShape.GetTriangleCount(); ++i)
    {
        ea::array<Math::Vector3, 3> triangle = bbShape.GetTriangle(i);
        bool res = bbShape.IsFacingOutside(triangle);
        INFO(i);
        REQUIRE(res);
    }
}
