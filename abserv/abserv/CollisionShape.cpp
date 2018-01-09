#include "stdafx.h"
#include "CollisionShape.h"
#include "Vector3.h"
#include "ConvexHull.h"
#include "BoundingBox.h"
#include "Sphere.h"
#include "HeightMap.h"

namespace Math {

void CollisionShape::SetConvexHull(const std::vector<Vector3>& vertices)
{
    shapeType_ = ShapeTypeConvexHull;
    hullShape_ = std::make_shared<ConvexHull>(vertices);
}

void CollisionShape::SetBox(const Vector3& min, const Vector3& max)
{
    shapeType_ = ShapeTypeBox;
    boxShape_ = std::make_shared<BoundingBox>(min, max);
}

void CollisionShape::SetBox(float min, float max)
{
    shapeType_ = ShapeTypeBox;
    boxShape_ = std::make_shared<BoundingBox>(min, max);
}

void CollisionShape::SetSphere(const Vector3& center, float radius)
{
    shapeType_ = ShapeTypeSphere;
    sphereShape_ = std::make_shared<Sphere>(center, radius);
}

BoundingBox CollisionShape::GetWorldBoundingBox(const Matrix4& transform) const
{
    switch (shapeType_)
    {
    case ShapeTypeBox:
        return boxShape_->Transformed(transform);
    case ShapeTypeSphere:
        return BoundingBox(sphereShape_->center_ - sphereShape_->radius_,
            sphereShape_->center_ + sphereShape_->radius_);
    case ShapeTypeConvexHull:
        return hullShape_->GetBoundingBox().Transformed(transform);
    }
    return BoundingBox();
}

}
