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
