#include "stdafx.h"
#include "HeightMap.h"
#include "MathUtils.h"
#include "Logger.h"

namespace Math {

HeightMap::HeightMap()
{
}

HeightMap::HeightMap(const std::vector<float>& data, const Point<int>& size) :
    numVertices_(size)
{
    heightData_ = data;
    ProcessData();
}

void HeightMap::ProcessData()
{
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

float HeightMap::GetHeight(const Vector3& world, const Matrix4& matrix) const
{
    // Get local
    Vector3 position = matrix.Inverse() * world;
    float xPos = (position.x_ / spacing_.x_) + ((float)numVertices_.x_ / 2.0f);
    float zPos = (position.z_ / spacing_.z_) + ((float)numVertices_.y_ / 2.0f);
    float xFrac = Fract(xPos);
    float zFrac = Fract(zPos);
    unsigned uxPos = (unsigned)xPos;
    unsigned uzPos = (unsigned)zPos;
    float h1, h2, h3;

    if (xFrac + zFrac >= 1.0f)
    {
        h1 = GetRawHeight(uxPos + 1, uzPos + 1) * spacing_.y_;
        h2 = GetRawHeight(uxPos, uzPos + 1) * spacing_.y_;
        h3 = GetRawHeight(uxPos + 1, uzPos) * spacing_.y_;
        xFrac = 1.0f - xFrac;
        zFrac = 1.0f - zFrac;
    }
    else
    {
        h1 = GetRawHeight(uxPos, uzPos) * spacing_.y_;
        h2 = GetRawHeight(uxPos + 1, uzPos) * spacing_.y_;
        h3 = GetRawHeight(uxPos, uzPos + 1) * spacing_.y_;
    }

    float h = h1 * (1.0f - xFrac - zFrac) + h2 * xFrac + h3 * zFrac;

    /// \todo This assumes that the terrain scene node is upright
    float result = matrix.Scaling().y_ * h + matrix.Translation().y_;
//    LOG_DEBUG << "X=" << position.x_ << " Y=" << position.y_ << " H=" << result << std::endl;
//    LOG_DEBUG << "X=" << (unsigned)xPos << " Y=" << (unsigned)zPos << " H=" << GetRawHeight((unsigned)xPos, (unsigned)zPos) << std::endl;
    return result;
}

Vector3 HeightMap::GetNormal(const Vector3& world, const Matrix4& matrix) const
{
    Vector3 position = matrix.Inverse() * world;
    float xPos = (position.x_ / spacing_.x_) + ((float)numVertices_.x_ / 2.0f);
    float zPos = (position.z_ / spacing_.z_) + ((float)numVertices_.y_ / 2.0f);
    float xFrac = Fract(xPos);
    float zFrac = Fract(zPos);
    Vector3 n1, n2, n3;

    if (xFrac + zFrac >= 1.0f)
    {
        n1 = GetRawNormal((unsigned)xPos + 1, (unsigned)zPos + 1);
        n2 = GetRawNormal((unsigned)xPos, (unsigned)zPos + 1);
        n3 = GetRawNormal((unsigned)xPos + 1, (unsigned)zPos);
        xFrac = 1.0f - xFrac;
        zFrac = 1.0f - zFrac;
    }
    else
    {
        n1 = GetRawNormal((unsigned)xPos, (unsigned)zPos);
        n2 = GetRawNormal((unsigned)xPos + 1, (unsigned)zPos);
        n3 = GetRawNormal((unsigned)xPos, (unsigned)zPos + 1);
    }

    Vector3 n = (n1 * (1.0f - xFrac - zFrac) + n2 * xFrac + n3 * zFrac).Normal();
    return matrix.Rotation() * n;
}

Point<int> HeightMap::WorldToHeightmap(const Vector3& world, const Matrix4& matrix)
{
    Vector3 pos = matrix.Inverse() * world;
    int xPos = (int)(pos.x_ / spacing_.x_ + 0.5f);
    int zPos = (int)(pos.z_ / spacing_.z_ + 0.5f);
    xPos = Clamp(xPos, 0, numVertices_.x_ - 1);
    zPos = Clamp(zPos, 0, numVertices_.y_ - 1);
    return Point<int>(xPos, zPos);
}

Vector3 HeightMap::HeightmapToWorld(const Point<int>& pixel, const Matrix4& matrix)
{
    Point<int> pos(pixel.x_, numVertices_.y_ - 1 - pixel.y_);
    float xPos = (float)(pos.x_ * spacing_.x_);
    float zPos = (float)(pos.y_ * spacing_.z_);
    Vector3 lPos(xPos, 0.0f, zPos);
    Vector3 wPos = matrix * lPos;
    wPos.y_ = GetHeight(wPos, matrix);

    return wPos;
}

}
