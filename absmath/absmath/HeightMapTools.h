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

#pragma once

#include <eastl.hpp>
#include "Shape.h"
#include "Point.h"
#include <functional>

namespace Math {

// Create height values from a 3D mesh
// \param[in] shape Source shape
// \param[in] targetWdth Desired width of height map. If 0 heightmaps width is the extends of the shape.
// \param[in] targetHeight Desired height of height map. If 0 heightmaps height is the extends of the shape.
// \param[out] width Width of height map
// \param[out] height Height of height map
// \param[out] minHeight Min height value
// \param[out] maxHeight Max height value
ea::vector<float> CreateHeightMapFromMesh(const Math::Shape& shape,
    int targetWdth, int targetHeight,
    int& width, int& height,
    float& minHeight, float& maxHeight);

// Create height values from an Image
// \param[in] data Image data
// \param[in] width Image width
// \param[in] height Image height
// \param[in] components Number of color components
// \param[in] spacing Height map spacing
// \param[in] patchSize Patch size
// \param[out] patchWorldSize Size of patch in world uints
// \param[out] numPatches Number of patches
// \param[out] numVertices Number of vertices
// \param[out] patchWorldOrigin Origin in world units
// \param[out] minHeight Min height value
// \param[out] maxHeight Max height value
ea::vector<float> CreateHeightMapFromImage(const unsigned char* data, int width, int height, int components,
    const Vector3& spacing, int patchSize,
    Vector2& patchWorldSize, IntVector2& numPatches, IntVector2& numVertices, Vector2& patchWorldOrigin,
    float& minHeight, float& maxHeight);

// Create a 3D mesh from a height map image
// \param[in] data Image data
// \param[in] width Image width
// \param[in] height Image height
// \param[in] components Number of color components
// \param[in] spacing Height map spacing
// \param[in] patchSize Patch size
// \param[in] onVertex Vertex callback
// \param[in] onTriangle Triangle callback
// \param[out] patchWorldSize Size of patch in world uints
// \param[out] numPatches Number of patches
// \param[out] numVertices Number of vertices
// \param[out] patchWorldOrigin Origin in world units
void CreateShapeFromHeightmapImage(const unsigned char* data, int width, int height, int components,
    const Vector3& spacing, int patchSize,
    const std::function<void(const Vector3& vertex)>& onVertex,
    const std::function<void(int i1, int i2, int i3)>& onTriangle,
    Vector2& patchWorldSize, IntVector2& numPatches, IntVector2& numVertices, Vector2& patchWorldOrigin);

}
