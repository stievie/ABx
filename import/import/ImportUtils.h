#pragma once

aiVector3D Vector3Add(const aiVector3D& lhs, const aiVector3D& rhs)
{
    return aiVector3D(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}
