/**
 * Copyright 2017-2020 Stefan Ascher
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

#include "CollisionShape.h"
#include "VectorMath.h"
#include <algorithm>
#include "Plane.h"
#include "Ray.h"
#include "Sphere.h"
#include <iostream>

namespace Math {

AbstractCollisionShape::~AbstractCollisionShape() = default;

ea::unique_ptr<Math::AbstractCollisionShape> AbstractCollisionShape::GetTranformedShapePtr(const Matrix4& matrix) const
{
    switch (shapeType_)
    {
    case Math::ShapeType::BoundingBox:
    {
        using BBoxShape = Math::CollisionShape<Math::BoundingBox>;
        return ea::make_unique<BBoxShape>(static_cast<const BBoxShape&>(*this), matrix);
    }
    case Math::ShapeType::Sphere:
    {
        using SphereShape = Math::CollisionShape<Math::Sphere>;
        return ea::make_unique<SphereShape>(static_cast<const SphereShape&>(*this), matrix);
    }
    case Math::ShapeType::ConvexHull:
    {
        using HullShape = Math::CollisionShape<Math::ConvexHull>;
        return ea::make_unique<HullShape>(static_cast<const HullShape&>(*this), matrix);
    }
    case Math::ShapeType::TriangleMesh:
    {
        using MeshShape = Math::CollisionShape<Math::TriangleMesh>;
        return ea::make_unique<MeshShape>(static_cast<const MeshShape&>(*this), matrix);
    }
    case Math::ShapeType::HeightMap:
    {
        using HeightShape = Math::CollisionShape<Math::HeightMap>;
        return ea::make_unique<HeightShape>(static_cast<const HeightShape&>(*this), matrix);
    }
    default:
        ASSERT_FALSE();
    }
}

bool AbstractCollisionShape::Collides(const AbstractCollisionShape& other, const Matrix4& transformation, const Vector3& velocity, Vector3& move) const
{
    switch (other.shapeType_)
    {
    case Math::ShapeType::BoundingBox:
    {
        using BBoxShape = Math::CollisionShape<Math::BoundingBox>;
        const BBoxShape& shape = static_cast<const BBoxShape&>(other);
        const Math::BoundingBox bbox = shape.Object().Transformed(transformation);
        return Collides(bbox, velocity, move);
    }
    case Math::ShapeType::Sphere:
    {
        using SphereShape = Math::CollisionShape<Math::Sphere>;
        const SphereShape& shape = static_cast<const SphereShape&>(other);
        const Math::Sphere sphere = shape.Object().Transformed(transformation);
        return Collides(sphere, velocity, move);
    }
    case Math::ShapeType::ConvexHull:
    {
        using HullShape = Math::CollisionShape<Math::ConvexHull>;
        const HullShape& shape = static_cast<const HullShape&>(other);
        const Math::ConvexHull hull = shape.Object().Transformed(transformation);
        return Collides(hull, velocity, move);
    }
    case Math::ShapeType::TriangleMesh:
    {
        using MeshShape = Math::CollisionShape<Math::TriangleMesh>;
        const MeshShape& shape = static_cast<const MeshShape&>(other);
        const Math::TriangleMesh mesh = shape.Object().Transformed(transformation);
        return Collides(mesh, velocity, move);
    }
    case Math::ShapeType::HeightMap:
    {
        using HeightShape = Math::CollisionShape<Math::HeightMap>;
        const HeightShape& shape = static_cast<const HeightShape&>(other);
        return Collides(shape.Object(), velocity, move);
    }
    default:
        ASSERT_FALSE();
    }
}

bool AbstractCollisionShape::GetManifold(CollisionManifold& manifold, const Matrix4& transformation) const
{
    // We know we are colliding with this object. this simplifies stuff.

    bool result = false;
    // Transform to world coordinates
    const Shape shape = GetShape();

    // World coordinates
    const Vector3& source = manifold.position;
    Vector3 normalizedVelocity = manifold.velocity.Normal();

    float distanceToTravel = manifold.velocity.Length();

    // Radius is 1 because the points are in our ellipsoid space
    const Sphere sphere(source, 1.0f);

    Vector3 sphereIntersectionPoint;
    Vector3 planeIntersectionPoint;
    Vector3 polyIntersectionPoint;

    float distToPlaneIntersection = 0.0f;
    float distToEllipsoidIntersection = 0.0f;

    for (size_t i = 0; i < shape.GetCount(); i += 3)
    {
        // One face world coordinates
        const Vector3 A = transformation * shape.GetVertex(i);
        const Vector3 B = transformation * shape.GetVertex(i + 1);
        const Vector3 C = transformation * shape.GetVertex(i + 2);

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

        sphereIntersectionPoint = source - planeNormal;

        PointClass pointClass = GetPointClass(sphereIntersectionPoint, planeOrigin, planeNormal);
        if (pointClass == PointClass::PlaneBack)
        {
            distToPlaneIntersection = IntersectsRayPlane(sphereIntersectionPoint, planeNormal, planeOrigin, planeNormal);
            planeIntersectionPoint = sphereIntersectionPoint + (distToPlaneIntersection * planeNormal);
        }
        else
        {
            distToPlaneIntersection = IntersectsRayPlane(sphereIntersectionPoint, normalizedVelocity, planeOrigin, planeNormal);
            planeIntersectionPoint = sphereIntersectionPoint + (distToPlaneIntersection * normalizedVelocity);
        }

        polyIntersectionPoint = planeIntersectionPoint;
        distToEllipsoidIntersection = distToPlaneIntersection;

        if (!IsPointInTriangle(planeIntersectionPoint, p1, p2, p3))
        {
            polyIntersectionPoint = GetClosestMatchingPointOnTriangle(p1, p2, p3, planeIntersectionPoint);
            // PolyPoint -> colliding object
            distToEllipsoidIntersection = IntersectsRaySphere(polyIntersectionPoint, -normalizedVelocity, source, 1.0f);
            if (distToEllipsoidIntersection > 0.0f)
            {
                sphereIntersectionPoint = polyIntersectionPoint + distToEllipsoidIntersection * -normalizedVelocity;
            }
        }

        if (IsPointInSphere(polyIntersectionPoint, source, 1.0f))
            manifold.stuck = true;

        // Update collision data if we hit something
        if ((distToEllipsoidIntersection > 0) && (distToEllipsoidIntersection <= distanceToTravel))
        {
            if (!manifold.foundCollision || (distToEllipsoidIntersection < manifold.nearestDistance))
            {
                manifold.nearestDistance = distToEllipsoidIntersection;
                manifold.nearestIntersectionPoint = sphereIntersectionPoint;
                manifold.nearestPolygonIntersectionPoint = polyIntersectionPoint;
                manifold.normal = planeNormal;
                manifold.foundCollision = true;
                result = true;
            }
        }
    }
    return result;
}

}
