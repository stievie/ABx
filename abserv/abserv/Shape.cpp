#include "stdafx.h"
#include "Shape.h"
#include "Vector4.h"

namespace Math {

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

}
