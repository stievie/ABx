#pragma once

template <typename T>
inline T Clamp(T value, T min, T max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

aiVector3D Vector3Add(const aiVector3D& lhs, const aiVector3D& rhs)
{
    return aiVector3D(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}
aiVector3D Vector3Sub(const aiVector3D& lhs, const aiVector3D& rhs)
{
    return aiVector3D(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}
aiVector3D Vector3Neg(const aiVector3D& v)
{
    return aiVector3D(-v.x, -v.y, -v.z);
}

aiVector3D CrossProduct(const aiVector3D& a, const aiVector3D& b)
{
    return{
        (a.y * b.z) - (a.z * b.y),
        (a.z * b.x) - (a.x * b.z),
        (a.x * b.y) - (a.y * b.x)
    };
}
