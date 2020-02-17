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

#include <absmath/Ray.h>
#include <absmath/BoundingBox.h>
#include <absmath/Sphere.h>

TEST_CASE("Ray", "[ray]")
{
    SECTION("Project")
    {
        Math::Vector3 origin(0.0f, 0.0f, 0.0f);
        Math::Vector3 direction(0.0f, 0.0f, 1.0f);
        Math::Ray ray(origin, direction);
        Math::Vector3 proj = ray.Project(Math::Vector3(0.0f, 0.0f, 5.0f));
        REQUIRE(proj.x_ == 0.0f);
        REQUIRE(proj.y_ == 0.0f);
        REQUIRE(proj.z_ == 5.0f);
    }
    SECTION("Distance")
    {
        Math::Vector3 origin(1.0f, 1.0f, 1.0f);
        Math::Vector3 direction(0.0f, 1.0f, 0.0f);
        Math::Ray ray(origin, direction);
        float dist = ray.Distance(Math::Vector3(5.0f, 1.0f, 5.0f));
        REQUIRE(dist == Approx(5.65685));
    }
}

TEST_CASE("Ray Hit", "[ray]")
{
    SECTION("BoundingBox")
    {
        Math::Vector3 origin(0.0f, 0.0f, 0.0f);
        Math::Vector3 direction(0.0f, 0.0f, 1.0f);
        Math::Ray ray(origin, direction);
        Math::BoundingBox bb(-1.0f, -1.0f, 2.0f, 1.0f, 1.0f, 4.0f);
        float dist = ray.HitDistance(bb);
        REQUIRE(dist == 2.0f);
    }
    SECTION("Sphere")
    {
        Math::Vector3 origin(0.0f, 0.0f, 0.0f);
        Math::Vector3 direction(0.0f, 0.0f, 1.0f);
        Math::Ray ray(origin, direction);
        Math::Sphere sphere(Math::Vector3(0.0f, 0.0f, 3.0f), 2.0f);
        float dist = ray.HitDistance(sphere);
        REQUIRE(dist == 1.0f);
    }
}
