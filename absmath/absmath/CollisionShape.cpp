#include "stdafx.h"
#include "CollisionShape.h"
#include "VectorMath.h"
#include <algorithm>
#include "Plane.h"
#include "Ray.h"
#include "Sphere.h"
#include <iostream>

namespace Math {

AbstractCollisionShape::~AbstractCollisionShape() = default;

bool AbstractCollisionShape::GetManifold(CollisionManifold& manifold, const Matrix4& transformation) const
{
    // We knopw we are colliding with this object. this simplifies stuff.

    bool result = false;
    // Transform to world coordinates
    const Shape shape = GetShape().Transformed(transformation);

    // World coordinates
    const Vector3& source = manifold.position;
    Vector3 normalizedVelocity = manifold.velocity.Normal();

    Vector3 planeIntersectionPoint;

    float distance = manifold.velocity.Length();
    float distToPlaneIntersection;

    // Radius is 1 because the points are in our ellipsoid space
    const Sphere sphere(source, 1.0f);

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

        PointClass pointClass = GetPointClass(sphereIntersectionPoint, planeOrigin, planeNormal);
        if (pointClass == PointClass::PlaneBack)
        {
            const Ray ray(sphereIntersectionPoint, planeNormal);
            const Plane plane(planeNormal, planeOrigin);
            distToPlaneIntersection = ray.HitDistance(plane);
            planeIntersectionPoint = sphereIntersectionPoint + (distToPlaneIntersection * planeNormal);
        }
        else
        {
            const Ray ray(sphereIntersectionPoint, normalizedVelocity);
            const Plane plane(planeNormal, planeOrigin);
            distToPlaneIntersection = ray.HitDistance(plane);
            planeIntersectionPoint = sphereIntersectionPoint + (distToPlaneIntersection * normalizedVelocity);
        }

        if (!IsPointInTriangle(planeIntersectionPoint, p1, p2, p3))
        {

        }

        if (sphere.IsInside(planeIntersectionPoint) == Intersection::Inside)
            manifold.stuck = true;

        // Update collision data if we hit something
        if (distToPlaneIntersection <= distance)
        {
            if (distToPlaneIntersection < manifold.nearestDistance)
            {
                manifold.nearestDistance = distToPlaneIntersection;
                manifold.nearestSphereIntersectionPoint = sphereIntersectionPoint;
                manifold.nearestPlaneIntersectionPoint = planeIntersectionPoint;
                result = true;
            }
        }
    }
    return result;
}

}
