#include "stdafx.h"
#include <catch.hpp>

#include "Ray.h"
#include "BoundingBox.h"
#include "Sphere.h"

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
        Math::Vector3 origin(0.0f, 0.0f, 0.0f);
        Math::Vector3 direction(0.0f, 0.0f, 1.0f);
        Math::Ray ray(origin, direction);
        float dist = ray.Distance(Math::Vector3(0.0f, 0.0f, 5.0f));
        // TODO: Shouldn't it be 5
        REQUIRE(dist == 0.0f);
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
