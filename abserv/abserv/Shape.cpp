#include "stdafx.h"
#include "Shape.h"
#include "Vector4.h"

namespace Math {

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
