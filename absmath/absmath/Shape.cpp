#include "stdafx.h"
#include "Shape.h"
#include "Vector4.h"
#include "VectorMath.h"

namespace Math {

void Shape::Reset()
{
    vertexData_.clear();
    vertexCount_ = 0;
    indexData_.clear();
    indexCount_ = 0;
}

void Shape::AddTriangle(unsigned i1, unsigned i2, unsigned i3)
{
    indexData_.push_back(i1);
    indexData_.push_back(i2);
    indexData_.push_back(i3);
    indexCount_ += 3;
}

bool Shape::IsFacingOutside(const std::array<Vector3, 3>& triangle) const
{
    // We only support convex shapes so calculate the normal and check if
    // it points outside.
    const Vector3 normal = GetTriangleNormal(triangle[0], triangle[1], triangle[2]);
    const Vector3 center = Center();
    const float r = normal.DotProduct(center);
    return r < 0.0f;
}

Vector3 Shape::Center() const
{
    Vector3 result;
    for (unsigned i = 0; i < GetCount(); ++i)
        result += GetVertex(i);
    result /= static_cast<float>(GetCount());
    return result;
}

Vector3 Shape::GetFarsetPointInDirection(const Vector3& v) const
{
    const Vector3& v0 = GetVertex(0);
    Vector3 worldVet = v0;
    Vector3 best(worldVet);
    float farest = worldVet.DotProduct(v);

    const unsigned count = GetCount();
    for (unsigned i = 1; i < count; ++i)
    {
        worldVet = GetVertex(i);
        float d = worldVet.DotProduct(v);
        if (farest < d)
        {
            best = worldVet;
            farest = d;
        }
    }
    return best;
}

Shape Shape::Transformed(const Matrix4& transformation) const
{
    Shape result(*this);
    result.matrix_ = transformation;
    return result;
}

}
