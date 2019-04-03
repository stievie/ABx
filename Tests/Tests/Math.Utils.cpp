#include "stdafx.h"
#include <catch.hpp>

#include "MathUtils.h"

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
