#include "stdafx.h"
#include <catch.hpp>

#include "Shape.h"
#include "BoundingBox.h"
#include "Sphere.h"

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
        std::array<Math::Vector3, 3> triangle = bbShape.GetTriangle(i);
        bool res = bbShape.IsFacingOutside(triangle);
        INFO(i);
        REQUIRE(res);
    }
}
