#include "stdafx.h"
#include "HeightMap.h"
#include "MathUtils.h"
#include "Sphere.h"
#include "BoundingBox.h"
#include "ConvexHull.h"
#include "Shape.h"

namespace Math {

HeightMap::HeightMap() :
    minHeight_(std::numeric_limits<float>::max()),
    maxHeight_(std::numeric_limits<float>::lowest())
{
}

HeightMap::HeightMap(const std::vector<float>& data, const Point<int>& size) :
    minHeight_(std::numeric_limits<float>::max()),
    maxHeight_(std::numeric_limits<float>::lowest()),
    numVertices_(size),
    heightData_(data)
{
    ProcessData();
}

void HeightMap::ProcessData()
{
    const unsigned points = (unsigned)(numVertices_.x_ * numVertices_.y_);
    const float* fData = heightData_.data();

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
    // TODO: +/- 16?
    const int _x = Clamp(x - 16, 0, numVertices_.x_ - 1);
    const int _z = Clamp(z + 16, 0, numVertices_.y_ - 1);
    return heightData_[_z * numVertices_.x_ + _x];
}

Vector3 HeightMap::GetRawNormal(int x, int z) const
{
    const float baseHeight = GetRawHeight(x, z);
    const float nSlope = GetRawHeight(x, z - 1) - baseHeight;
    const float neSlope = GetRawHeight(x + 1, z - 1) - baseHeight;
    const float eSlope = GetRawHeight(x + 1, z) - baseHeight;
    const float seSlope = GetRawHeight(x + 1, z + 1) - baseHeight;
    const float sSlope = GetRawHeight(x, z + 1) - baseHeight;
    const float swSlope = GetRawHeight(x - 1, z + 1) - baseHeight;
    const float wSlope = GetRawHeight(x - 1, z) - baseHeight;
    const float nwSlope = GetRawHeight(x - 1, z - 1) - baseHeight;
    const float up = 0.5f * (spacing_.x_ + spacing_.z_);

    return (Vector3(0.0f, up, nSlope) +
        Vector3(-neSlope, up, neSlope) +
        Vector3(-eSlope, up, 0.0f) +
        Vector3(-seSlope, up, -seSlope) +
        Vector3(0.0f, up, -sSlope) +
        Vector3(swSlope, up, -swSlope) +
        Vector3(wSlope, up, 0.0f) +
        Vector3(nwSlope, up, nwSlope)).Normal();
}

float HeightMap::GetHeight(const Vector3& world) const
{
    // Get local
    const Vector3 position = matrix_.Inverse() * world;
    const float xPos = (position.x_ / spacing_.x_) + ((float)numVertices_.x_ / 2.0f);
    const float zPos = (position.z_ / spacing_.z_) + ((float)numVertices_.y_ / 2.0f);
    float xFrac = Fract(xPos);
    float zFrac = Fract(zPos);
    const unsigned uxPos = static_cast<unsigned>(xPos);
    const unsigned uzPos = static_cast<unsigned>(zPos);
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

    const float h = h1 * (1.0f - xFrac - zFrac) + h2 * xFrac + h3 * zFrac;

    /// \todo This assumes that the terrain scene node is upright
    float result = matrix_.Scaling().y_ * h + matrix_.Translation().y_;
#ifdef _DEBUG
//    LOG_DEBUG << "X=" << position.x_ << " Z=" << position.z_ << " H=" << result << std::endl;
//    LOG_DEBUG << "X=" << (unsigned)xPos << " Y=" << (unsigned)zPos << " H=" << GetRawHeight((unsigned)xPos, (unsigned)zPos) << std::endl;
#endif
    return result;
}

Vector3 HeightMap::GetNormal(const Vector3& world) const
{
    const Vector3 position = matrix_.Inverse() * world;
    const float xPos = (position.x_ / spacing_.x_) + ((float)numVertices_.x_ / 2.0f);
    const float zPos = (position.z_ / spacing_.z_) + ((float)numVertices_.y_ / 2.0f);
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

    const Vector3 n = (n1 * (1.0f - xFrac - zFrac) + n2 * xFrac + n3 * zFrac).Normal();
    return matrix_.Rotation() * n;
}

bool HeightMap::Collides(const Sphere& b2, const Vector3& velocity, Vector3& move) const
{
    return b2.Collides(*this, velocity, move);
}

bool HeightMap::Collides(const BoundingBox& b2, const Vector3& velocity, Vector3& move) const
{
    return b2.Collides(*this, velocity, move);
}

bool HeightMap::Collides(const ConvexHull& b2, const Vector3& velocity, Vector3& move) const
{
    return b2.Collides(*this, velocity, move);
}

bool HeightMap::Collides(const HeightMap&, const Vector3&, Vector3&) const
{
    // Can not collide Heightmap with Heightmap
    assert(false);
    return false;
}

Shape HeightMap::GetShape() const
{
    Shape s;
    s.vertexData_.resize(numVertices_.x_ + numVertices_.y_);
    for (int x = 0; x < numVertices_.x_; ++x)
    {
        for (int z = 0; z < numVertices_.y_; ++z)
        {
            float fy = GetRawHeight(x, z);
            float fx = (float)x - (float)numVertices_.x_ / 2.0f;
            float fz = (float)z - (float)numVertices_.y_ / 2.0f;
            s.vertexData_[z * numVertices_.y_ + x] = {
                fx,
                fy,
                fz
            };
        }
    }

    // Create index data
    for (int x = 0; x < numVertices_.x_ - 1; ++x)
    {
        for (int z = 0; z < numVertices_.y_ - 1; ++z)
        {
            /*
            Normal edge:
            +----+----+
            |\ 1 |\   |
            | \  | \  |
            |  \ |  \ |
            | 2 \|   \|
            +----+----+
            */
            {
                // First triangle
                int i1 = z * numVertices_.x_ + x;
                int i2 = (z * numVertices_.x_) + x + 1;
                int i3 = (z + 1) * numVertices_.x_ + (x + 1);
                s.AddTriangle(i1, i2, i3);
            }

            {
                // Second triangle
                int i3 = (z + 1) * numVertices_.x_ + (x + 1);
                int i2 = (z + 1) * numVertices_.x_ + x;
                int i1 = z * numVertices_.x_ + x;
                s.AddTriangle(i3, i2, i1);
            }
        }
    }

    return s;
}

Point<int> HeightMap::WorldToHeightmap(const Vector3& world)
{
    const Vector3 pos = matrix_.Inverse() * world;
    int xPos = (int)(pos.x_ / spacing_.x_ + 0.5f);
    int zPos = (int)(pos.z_ / spacing_.z_ + 0.5f);
    xPos = Clamp(xPos, 0, numVertices_.x_ - 1);
    zPos = Clamp(zPos, 0, numVertices_.y_ - 1);
    return Point<int>(xPos, zPos);
}

Vector3 HeightMap::HeightmapToWorld(const Point<int>& pixel)
{
    const Point<int> pos(pixel.x_, numVertices_.y_ - 1 - pixel.y_);
    const float xPos = (float)(pos.x_ * spacing_.x_);
    const float zPos = (float)(pos.y_ * spacing_.z_);
    const Vector3 lPos(xPos, 0.0f, zPos);
    Vector3 wPos = matrix_ * lPos;
    wPos.y_ = GetHeight(wPos);

    return wPos;
}

}
