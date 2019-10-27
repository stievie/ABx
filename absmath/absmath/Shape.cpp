#include "stdafx.h"
#include "Shape.h"
#include "Vector4.h"
#include <execution>

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

Vector3 Shape::GetFarsetPointInDirection(const Vector3& v) const
{
    const Vector3& v0 = GetVertex(0);
    Vector3 worldVet = matrix_ * v0;
    Vector3 best(worldVet);
    float farest = worldVet.DotProduct(v);

    const unsigned count = GetCount();
    for (unsigned i = 1; i < count; ++i)
    {
        worldVet = matrix_ * GetVertex(i);
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
    Shape result;
    result.vertexCount_ = vertexCount_;
    result.indexData_ = indexData_;
    result.indexCount_ = indexCount_;
    result.vertexData_.reserve(vertexCount_);

#if 0
    std::for_each(std::execution::par_unseq,
        vertexData_.begin(), vertexData_.end(),
        [&](const auto& item)
    {
        result.vertexData_.push_back(transformation * item);
    });
#endif

    for (const auto& v : vertexData_)
        result.vertexData_.push_back(transformation * v);

    return result;
}

Shape Shape::Transformed() const
{
    return Transformed(matrix_);
}

}
