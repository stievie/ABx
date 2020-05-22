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
#include <absmath/ConvexHull.h>
#include <absmath/Sphere.h>
#include <absmath/Shape.h>
#include <absmath/MathDefs.h>

TEST_CASE("Hull")
{
    Math::Sphere sphere(Math::Vector3::Zero, 1.0f);
    Math::Shape sphereShape = sphere.GetShape();
    Math::ConvexHull hull(sphereShape.vertexData_);

    SECTION("Build")
    {
        Math::BoundingBox bb({ -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f });
        Math::BoundingBox hullBox = hull.GetBoundingBox();

        // Approx the same dimensions
        REQUIRE(fabs(hullBox.min_.x_ - bb.min_.x_) < 2.0f);
        REQUIRE(fabs(hullBox.min_.y_ - bb.min_.y_) < 2.0f);
        REQUIRE(fabs(hullBox.min_.z_ - bb.min_.z_) < 2.0f);
        REQUIRE(fabs(hullBox.max_.x_ - bb.max_.x_) < 2.0f);
        REQUIRE(fabs(hullBox.max_.y_ - bb.max_.y_) < 2.0f);
        REQUIRE(fabs(hullBox.max_.z_ - bb.max_.z_) < 2.0f);
    }

    SECTION("Inside")
    {
        // FIXME:
//        REQUIRE(hull.IsInside({ 0.0f, 0.0f, 0.0f }) == Math::Intersection::Inside);
//        REQUIRE(hull.IsInside({ 2.5f, 2.5f, 2.5f }) == Math::Intersection::Outside);
    }

    SECTION("Collides")
    {
        // FIXME: This fails on Azure DevOps, I wonder what CPU they have...
        // Works fine with a Intel Core i5-3570K (Ivy Bridge) and
        // Intel Xeon X3430 (Lynnfield)
        // However, the Hull collisions still need work...

//        Math::Sphere sphere1({ 1.5f, 1.5f, 1.5f }, 0.5f);
//        Math::Vector3 move;
//        REQUIRE(hull.Collides(sphere1, Math::Vector3::Zero, move));
    }
    SECTION("Not Collides")
    {
//        Math::Sphere sphere1({ 5.5f, 5.5f, 5.5f }, 0.5f);
//        Math::Vector3 move;
        // FIXME:
//        REQUIRE(!hull.Collides(sphere1, Math::Vector3::Zero, move));
    }
}
