#include "stdafx.h"
#include <catch.hpp>
#include "ConvexHull.h"
#include "Sphere.h"
#include "Shape.h"
#include "MathDefs.h"

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
//        REQUIRE(hull.IsInside({ 0.0f, 0.0f, 0.0f }) == Math::INSIDE);
//        REQUIRE(hull.IsInside({ 2.5f, 2.5f, 2.5f }) == Math::OUTSIDE);
    }

    SECTION("Collides")
    {
        Math::Sphere sphere1({ 1.5f, 1.5f, 1.5f }, 0.5f);
        Math::Vector3 move;
        REQUIRE(hull.Collides(sphere1, Math::Vector3::Zero, move));
    }
    SECTION("Not Collides")
    {
        Math::Sphere sphere1({ 5.5f, 5.5f, 5.5f }, 0.5f);
        Math::Vector3 move;
        // FIXME:
//        REQUIRE(!hull.Collides(sphere1, Math::Vector3::Zero, move));
    }
}
