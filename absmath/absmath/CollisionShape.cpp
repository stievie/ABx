#include "stdafx.h"
#include "CollisionShape.h"
#include "VectorMath.h"
#include <algorithm>
#include "Plane.h"
#include "Ray.h"
#include "Sphere.h"
#include <iostream>
#include "BoundingBox.h"

namespace Math {

AbstractCollisionShape::~AbstractCollisionShape() = default;

bool AbstractCollisionShape::GetManifold(CollisionManifold& manifold, const Matrix4& transformation) const
{
    // We know we are colliding with this object. this simplifies stuff.

    bool result = false;
    // Transform to world coordinates
    const Shape shape = GetShape().Transformed(transformation);

    // World coordinates
    const Vector3& source = manifold.position;
    manifold.normalizedVelocity = manifold.velocity.Normal();

    float distance = manifold.velocity.Length();

    // Radius is 1 because the points are in our ellipsoid space
    const Sphere sphere(source, 1.0f);
    const BoundingBox bb(source - 1.0f, source + 1.0f);

    for (unsigned i = 0; i < shape.GetCount(); i += 3)
    {
        // One face world coordinates
        const Vector3& A = shape.GetVertex(i);
        const Vector3& B = shape.GetVertex(i + 1);
        const Vector3& C = shape.GetVertex(i + 2);

        // Scale the triangle to ellipsoid space
        const Vector3 p1 = A / manifold.radius;
        const Vector3 p2 = B / manifold.radius;
        const Vector3 p3 = C / manifold.radius;

        Vector3 planeOrigin = p1;
        Vector3 v1 = p2 - p1;
        Vector3 v2 = p3 - p1;

        if (v1.Equals(Vector3::Zero) || v2.Equals(Vector3::Zero))
            continue;

        const Vector3 planeNormal = v1.CrossProduct(v2).Normal();

        Vector3 sphereIntersectionPoint = source - planeNormal;

        float distToIntersection;
        Vector3 intersectionPoint;
        PointClass pointClass = GetPointClass(sphereIntersectionPoint, planeOrigin, planeNormal);
        if (pointClass == PointClass::PlaneBack)
        {
            const Ray ray(sphereIntersectionPoint, planeNormal);
            const Plane plane(planeNormal, planeOrigin);
            distToIntersection = ray.HitDistance(plane);
            intersectionPoint = sphereIntersectionPoint + (distToIntersection * planeNormal);
        }
        else
        {
            const Ray ray(sphereIntersectionPoint, manifold.normalizedVelocity);
            const Plane plane(planeNormal, planeOrigin);
            distToIntersection = ray.HitDistance(plane);
            intersectionPoint = sphereIntersectionPoint + (distToIntersection * manifold.normalizedVelocity);
        }

        if (!IsPointInTriangle(intersectionPoint, p1, p2, p3))
        {
            const Vector3 polyPoint = GetClosestPointOnTriangle(p1, p2, p3, intersectionPoint);
            // PolyPoint -> colliding object
            const Ray ray(polyPoint, -manifold.normalizedVelocity);
            float dist = ray.HitDistance(sphere);
            if (!IsInfinite(dist))
            {
                distToIntersection = dist;
                intersectionPoint = intersectionPoint + (distToIntersection * -manifold.normalizedVelocity);
            }
        }

        if (sphere.IsInside(intersectionPoint) == Intersection::Inside)
            manifold.stuck = true;

        // Update collision data if we hit something
        if (distToIntersection <= distance)
        {
            if (distToIntersection < manifold.distance)
            {
                manifold.distance = distToIntersection;
                manifold.nearestSphereIntersectionPoint = sphereIntersectionPoint;
                manifold.intersectionPoint = intersectionPoint;
                manifold.normal = planeNormal;
                result = true;
            }
        }
    }
    return result;
}

}
