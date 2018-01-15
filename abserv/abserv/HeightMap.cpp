#include "stdafx.h"
#include "HeightMap.h"
#include "MathUtils.h"

namespace Math {

HeightMap::HeightMap()
{
}

HeightMap::HeightMap(const std::vector<float>& data, const Point<int>& size) :
    numVertices_(size)
{
    heightData_ = data;
    unsigned points = (unsigned)(numVertices_.x_ * numVertices_.y_);
    float* fData = heightData_.data();

    minHeight_ = maxHeight_ = fData[0];
    for (unsigned i = 1; i < points; ++i)
    {
        minHeight_ = std::min(minHeight_, fData[i]);
        maxHeight_ = std::max(maxHeight_, fData[i]);
    }

    const Vector3 localAabbMin(0.0f, minHeight_, 0.0f);
    const Vector3 localAabbMax((float)numVertices_.x_, maxHeight_, (float)numVertices_.y_);
    const Vector3 halfExtends = (localAabbMax - localAabbMin) * 0.5f;

    boundingBox_.min_ = -halfExtends;
    boundingBox_.max_ = halfExtends;
}

HeightMap::~HeightMap()
{
}

float HeightMap::GetRawHeight(int x, int z) const
{
    if (!heightData_.size())
        return 0.0f;
    x = Clamp(x, 0, numVertices_.x_ - 1);
    z = Clamp(z, 0, numVertices_.y_ - 1);
    return heightData_[z * numVertices_.x_ + x];
}

Vector3 HeightMap::GetRawNormal(int x, int z) const
{
    float baseHeight = GetRawHeight(x, z);
    float nSlope = GetRawHeight(x, z - 1) - baseHeight;
    float neSlope = GetRawHeight(x + 1, z - 1) - baseHeight;
    float eSlope = GetRawHeight(x + 1, z) - baseHeight;
    float seSlope = GetRawHeight(x + 1, z + 1) - baseHeight;
    float sSlope = GetRawHeight(x, z + 1) - baseHeight;
    float swSlope = GetRawHeight(x - 1, z + 1) - baseHeight;
    float wSlope = GetRawHeight(x - 1, z) - baseHeight;
    float nwSlope = GetRawHeight(x - 1, z - 1) - baseHeight;
    float up = 0.5f * (spacing_.x_ + spacing_.z_);

    return (Vector3(0.0f, up, nSlope) +
        Vector3(-neSlope, up, neSlope) +
        Vector3(-eSlope, up, 0.0f) +
        Vector3(-seSlope, up, -seSlope) +
        Vector3(0.0f, up, -sSlope) +
        Vector3(swSlope, up, -swSlope) +
        Vector3(wSlope, up, 0.0f) +
        Vector3(nwSlope, up, nwSlope)).Normal();
}

}
