/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "HeightMap.h"
#include "MathUtils.h"
#include "Sphere.h"
#include "BoundingBox.h"
#include "ConvexHull.h"
#include "Shape.h"
#include <sa/Assert.h>

namespace Math {

HeightMap::HeightMap() :
    minHeight_(std::numeric_limits<float>::max()),
    maxHeight_(std::numeric_limits<float>::lowest())
{
    inverseMatrix_ = matrix_.Inverse();
}

void HeightMap::ProcessData()
{
    const unsigned points = static_cast<unsigned>(numVertices_.x_ * numVertices_.y_);

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
    const int _x = Clamp(x, 0, numVertices_.x_ - 1);
    const int _z = Clamp(z, 0, numVertices_.y_ - 1);
    const size_t offset = static_cast<size_t>(_z * numVertices_.x_ + _x);
    return heightData_[offset];
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
    const Vector3 position = inverseMatrix_ * world;
    const float xPos = (position.x_ - patchWorldOrigin_.x_) / spacing_.x_;
    const float zPos = (position.z_ - patchWorldOrigin_.y_) / spacing_.z_;
    float xFrac = Fract(xPos);
    float zFrac = Fract(zPos);
    const unsigned uxPos = static_cast<unsigned>(xPos);
    const unsigned uzPos = static_cast<unsigned>(zPos);
    float h1, h2, h3;

    if (xFrac + zFrac >= 1.0f)
    {
        h1 = GetRawHeight(uxPos + 1, uzPos + 1);
        h2 = GetRawHeight(uxPos, uzPos + 1);
        h3 = GetRawHeight(uxPos + 1, uzPos);
        xFrac = 1.0f - xFrac;
        zFrac = 1.0f - zFrac;
    }
    else
    {
        h1 = GetRawHeight(uxPos, uzPos);
        h2 = GetRawHeight(uxPos + 1, uzPos);
        h3 = GetRawHeight(uxPos, uzPos + 1);
    }

    // 2nd Layer may have -inf for undefined heights
    if (IsNegInfinite(h1) || IsNegInfinite(h2) || IsNegInfinite(h3))
        return -M_INFINITE;

    const float h = h1 * (1.0f - xFrac - zFrac) + h2 * xFrac + h3 * zFrac;

    return matrix_.Scaling().y_ * h + matrix_.Translation().y_;
}

Vector3 HeightMap::GetNormal(const Vector3& world) const
{
    const Vector3 position = inverseMatrix_ * world;
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

void HeightMap::SetMatrix(const Matrix4& matrix)
{
    matrix_ = matrix;
    inverseMatrix_ = matrix_.Inverse();
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
    ASSERT_FALSE();
}

Shape HeightMap::GetShape() const
{
    Shape s;
    s.vertexData_.resize((size_t)numVertices_.x_ + (size_t)numVertices_.y_);
    const float offsetX = ((float)numVertices_.x_ * 0.5f);
    const float offsetY = ((float)numVertices_.y_ * 0.5f);
    for (int z = 0; z < numVertices_.y_; ++z)
    {
        for (int x = 0; x < numVertices_.x_; ++x)
        {
            float fy = GetRawHeight(x, z);
            float fx = static_cast<float>(x) - offsetX;
            float fz = static_cast<float>(z) - offsetY;
            s.vertexData_[(size_t)z * (size_t)numVertices_.y_ + (size_t)x] = {
                fx * spacing_.x_,
                fy * spacing_.y_,
                fz * spacing_.z_
            };
        }
    }

    // Create index data
    for (int z = 0; z < numVertices_.y_ - 1; ++z)
    {
        for (int x = 0; x < numVertices_.x_ - 1; ++x)
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
    const Vector3 pos = inverseMatrix_ * world;
    int xPos = static_cast<int>(pos.x_ / spacing_.x_ + 0.5f);
    int zPos = static_cast<int>(pos.z_ / spacing_.z_ + 0.5f);
    xPos = Clamp(xPos, 0, numVertices_.x_ - 1);
    zPos = Clamp(zPos, 0, numVertices_.y_ - 1);
    return Point<int>(xPos, zPos);
}

Vector3 HeightMap::HeightmapToWorld(const Point<int>& pixel)
{
    const Point<int> pos(pixel.x_, numVertices_.y_ - 1 - pixel.y_);
    const float xPos = static_cast<float>(pos.x_ * spacing_.x_);
    const float zPos = static_cast<float>(pos.y_ * spacing_.z_);
    const Vector3 lPos(xPos, 0.0f, zPos);
    Vector3 wPos = matrix_ * lPos;
    wPos.y_ = GetHeight(wPos);

    return wPos;
}

}
