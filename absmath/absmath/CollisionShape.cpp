#include "stdafx.h"
#include "CollisionShape.h"
#include "VectorMath.h"

namespace Math {

AbstractCollisionShape::~AbstractCollisionShape() = default;

void AbstractCollisionShape::GetManifold(CollisionManifold& manifold) const
{
    Shape shape = GetShape();

    const Vector3& source = manifold.position;
    Vector3 normalizedVelocity = manifold.velocity.Normal();

    Vector3 planeIntersectionPoint;
    Vector3 polyIntersectionPoint;

    float distance = manifold.velocity.Length();
    float distToPlaneIntersection;
    float distToEllipsoidIntersection;

    for (unsigned i = 0; i < shape.GetCount(); i += 3)
    {
        // One face
        Vector3 A = shape.GetVertex(i);
        Vector3 B = shape.GetVertex(i + 1);
        Vector3 C = shape.GetVertex(i + 2);

        Vector3 p1 = A / manifold.radius;
        Vector3 p2 = B / manifold.radius;
        Vector3 p3 = C / manifold.radius;

        Vector3 planeOrigin = p1;
        Vector3 v1 = p2 - p1;
        Vector3 v2 = p3 - p1;

        if (v1.Equals(Vector3::Zero) || v2.Equals(Vector3::Zero))
            continue;

        Vector3 planeNormal = WEdge(v1, v2).Normal();

        Vector3 spherePoint = source - planeNormal;

        PointClass pointClass = GetPointClass(spherePoint, planeOrigin, planeNormal);
        if (pointClass == PointClass::PlaneBack)
        {
            distToPlaneIntersection = IntersectsRayPlane(Ray(spherePoint, planeNormal), planeOrigin, planeNormal);
            planeIntersectionPoint = spherePoint + (distToPlaneIntersection * planeNormal);
        }
        else
        {
            distToPlaneIntersection = IntersectsRayPlane(Ray(spherePoint, normalizedVelocity), planeOrigin, planeNormal);
            planeIntersectionPoint = spherePoint + (distToPlaneIntersection * normalizedVelocity);
        }

        polyIntersectionPoint = planeIntersectionPoint;
        distToEllipsoidIntersection = distToPlaneIntersection;

        if (!IsPointInTriangle(planeIntersectionPoint, p1, p2, p3))
        {
            polyIntersectionPoint = GetClosestPointOnTriangle(p1, p2, p3, planeIntersectionPoint);
            distToEllipsoidIntersection = IntersectsRaySphere(Ray(polyIntersectionPoint, -normalizedVelocity),
                Sphere(source, 1.0f));
            if (distToEllipsoidIntersection > 0.0f)
            {
                planeIntersectionPoint = polyIntersectionPoint + distToEllipsoidIntersection * -normalizedVelocity;
            }
        }

        if (IsPointInSphere(polyIntersectionPoint, Sphere(source, 1.0f)))
        {
            manifold.stuck = true;
        }

        if ((distToEllipsoidIntersection > 0.0f) && (distToEllipsoidIntersection <= distance))
        {
            if (distToEllipsoidIntersection < manifold.nearestDistance)
            {
                manifold.nearestDistance = distToEllipsoidIntersection;
                manifold.nearestIntersectionPoint = planeIntersectionPoint;
                manifold.nearestPolygonIntersectionPoint = polyIntersectionPoint;
            }
        }
    }
}

}
