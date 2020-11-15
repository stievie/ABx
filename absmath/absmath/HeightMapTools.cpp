/**
 * Copyright 2020 Stefan Ascher
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

#include "HeightMapTools.h"
#include "VectorMath.h"

namespace Math {

ea::vector<float> CreateHeightMapFromMesh(const Math::Shape& shape,
    int& width, int& height,
    float& minHeight, float& maxHeight)
{
    const Math::Vector3 minX = shape.GetFarsetPointInDirection(-Math::Vector3::UnitX);
    const Math::Vector3 maxX = shape.GetFarsetPointInDirection(Math::Vector3::UnitX);
    const Math::Vector3 minZ = shape.GetFarsetPointInDirection(-Math::Vector3::UnitZ);
    const Math::Vector3 maxZ = shape.GetFarsetPointInDirection(Math::Vector3::UnitZ);

    const Math::Vector3 minHeightVertex = shape.GetFarsetPointInDirection(-Math::Vector3::UnitY);
    const Math::Vector3 maxHeightVertex = shape.GetFarsetPointInDirection(Math::Vector3::UnitY);
    minHeight = minHeightVertex.y_;
    maxHeight = maxHeightVertex.y_;

    width = (int)std::ceil(maxX.x_ - minX.x_);
    height = (int)std::ceil(maxZ.z_ - minZ.z_);

    ea::vector<float> heights;
    heights.resize((size_t)width * (size_t)height);
    ea::fill(heights.begin(), heights.end(), std::numeric_limits<float>::min());

    for (const auto& v : shape.vertexData_)
    {
        const int x = static_cast<int>(v.x_ - minX.x_);
        const int y = height - static_cast<int>(v.z_ - minZ.z_);

        if (x >= width)
            continue;
        if (y >= height)
            continue;

        const size_t index = (size_t)y * (size_t)width + (size_t)x;
        if (heights[index] < v.y_)
            heights[index] = v.y_;
    }

    // 2nd pass to fill triangles
    for (size_t t = 0; t < shape.GetTriangleCount(); ++t)
    {
        const auto triangle = shape.GetTriangle(t);

        const int minTriangleX = (int)floor(std::min(std::min(triangle[0].x_, triangle[1].x_), triangle[2].x_));
        const int minTriangleZ = (int)floor(std::min(std::min(triangle[0].z_, triangle[1].z_), triangle[2].z_));
        const int maxTriangleX = (int)ceil(std::max(std::max(triangle[0].x_, triangle[1].x_), triangle[2].x_));
        const int maxTriangleZ = (int)ceil(std::max(std::max(triangle[0].z_, triangle[1].z_), triangle[2].z_));

        for (int y = minTriangleZ; y <= maxTriangleZ; ++y)
        {
            for (int x = minTriangleX; x < maxTriangleX; ++x)
            {
                const Math::Vector3 point = { (float)x, triangle[0].y_, (float)y };
                if (Math::IsPointInTriangle(point, triangle[0], triangle[1], triangle[2]))
                {
                    const int posX = static_cast<int>(x - minX.x_);
                    const int posY = height - static_cast<int>(y - minZ.z_);

                    if (posX >= width)
                        continue;
                    if (posY >= height)
                        continue;

                    const Math::Vector3 triPoint = shape.GetClosestPointOnTriangle(triangle,
                        { (float)x, maxHeight, (float)y });

                    const size_t index = (size_t)posY * (size_t)width + (size_t)posX;

                    if (heights[index] < triPoint.y_)
                        heights[index] = triPoint.y_;
                }
            }
        }
    }

    return heights;
}

ea::vector<float> CreateHeightMapFromImage(const unsigned char* data, int width, int height, int components,
    const Vector3& spacing, int patchSize,
    Point<float>& patchWorldSize, Point<int>& numPatches, Point<int>& numVertices, Point<float>& patchWorldOrigin,
    float& minHeight, float& maxHeight)
{
    patchWorldSize = { spacing.x_ * (float)patchSize, spacing.z_ * (float)patchSize };
    numPatches = { (width - 1) / patchSize, (height - 1) / patchSize };
    numVertices = { numPatches.x_ * patchSize + 1, numPatches.y_ * patchSize + 1 };
    patchWorldOrigin = { -0.5f * (float)numPatches.x_ * patchWorldSize.x_, -0.5f * (float)numPatches.y_ * patchWorldSize.y_ };

    int imgRow = width * components;
    auto getHeight = [&](int x, int z) -> float
    {
        if (!data)
            return 0.0f;

        // From bottom to top
        int offset = imgRow * (numVertices.y_ - 1 - z) + components * x;

        if (components == 1)
            return (float)data[offset];

        // If more than 1 component, use the green channel for more accuracy
        return (float)data[offset] +
            (float)data[offset + 1] / 256.0f;
    };

    minHeight = std::numeric_limits<float>::max();
    maxHeight = std::numeric_limits<float>::lowest();

    ea::vector<float> heightData;
    heightData.resize((size_t)numVertices.x_ * (size_t)numVertices.y_);
    for (int y = 0; y < numVertices.y_; ++y)
    {
        for (int x = 0; x < numVertices.x_; ++x)
        {
            float fy = getHeight(x, y) * spacing.y_;
            heightData[(size_t)y * (size_t)numVertices.x_ + (size_t)x] = fy;
            if (minHeight > fy)
                minHeight = fy;
            if (maxHeight < fy)
                maxHeight = fy;
        }
    }
    return heightData;
}

void CreateShapeFromHeightmapImage(const unsigned char* data, int width, int height, int components,
    const Vector3& spacing, int patchSize,
    const std::function<void(const Vector3& vertex)>& onVertex,
    const std::function<void(int i1, int i2, int i3)>& onTriangle,
    Point<float>& patchWorldSize, Point<int>& numPatches, Point<int>& numVertices, Point<float>& patchWorldOrigin)
{
    // Image size - 1 sould be a multiple of patchSize, otherwise it cuts off some pixels.
    patchWorldSize = { spacing.x_ * (float)patchSize, spacing.z_ * (float)patchSize };
    numPatches = { (width - 1) / patchSize, (height - 1) / patchSize };
    numVertices = { numPatches.x_ * patchSize + 1, numPatches.y_ * patchSize + 1 };
    patchWorldOrigin = { -0.5f * (float)numPatches.x_ * patchWorldSize.x_, -0.5f * (float)numPatches.y_ * patchWorldSize.y_ };

    int imgRow = width * components;
    auto getHeight = [&](int x, int z) -> float
    {
        if (!data)
            return 0.0f;

        // From bottom to top
        int offset = imgRow * (numVertices.y_ - 1 - z) + components * x;

        if (components == 1)
            return (float)data[offset];

        // If more than 1 component, use the green channel for more accuracy
        return (float)data[offset] +
            (float)data[offset + 1] / 256.0f;
    };

    const float offsetX = ((float)numVertices.x_ * 0.5f);
    const float offsetY = ((float)numVertices.y_ * 0.5f);
    for (int y = 0; y < numVertices.y_; ++y)
    {
        for (int x = 0; x < numVertices.x_; ++x)
        {
            float fy = getHeight(x, y);
            float fx = (float)x - offsetX;
            float fz = (float)y - offsetY;
            if (onVertex)
                onVertex({ fx * spacing.x_, fy * spacing.y_, fz * spacing.z_ });
        }
    }

    // Create index data
    for (int y = 0; y < numVertices.y_ - 1; ++y)
    {
        for (int x = 0; x < numVertices.x_ - 1; ++x)
        {
            /*
                x+1,y
        x,y +----+----+
            | 1 /|(3)/|
            |  / |  / |
            | /  | /  |
            |/ 2 |/(4)|
      x,y+1 +----+----+
              x+1,y+1
            */
            {
                // First triangle
                int i1 = (y + 1) * numVertices.x_ + x;
                int i2 = y * numVertices.x_ + x;
                int i3 = (y * numVertices.x_) + x + 1;
                if (onTriangle)
                    onTriangle(i3, i2, i1);
            }

            {
                // Second triangle
                int i1 = y * numVertices.x_ + x + 1;
                int i2 = (y + 1) * numVertices.x_ + (x + 1);
                int i3 = (y + 1) * numVertices.x_ + x;
                if (onTriangle)
                    onTriangle(i3, i2, i1);
            }
        }
    }
}

}
