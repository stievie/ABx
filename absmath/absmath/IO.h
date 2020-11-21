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

#include "Shape.h"
#include "BoundingBox.h"
#include "Point.h"

namespace IO {

// Load a heightmap (.hm) file
// \param[in] name Filename
// \param[out] patchSize Patch size
// \param[out] patchWorldSize Size of patch in world uints
// \param[out] numPatches Number of patches
// \param[out] numVertices Number of vertices
// \param[out] patchWorldOrigin Origin in world units
// \param[out] minHeight Min height value
// \param[out] maxHeight Max height value
// \return The height values
ea::vector<float> LoadHeightmap(const std::string& name,
    int& patchSize,
    Math::Vector2& patchWorldSize, Math::IntVector2& numPatches,
    Math::IntVector2& numVertices, Math::Vector2& patchWorldOrigin,
    float& minHeight, float& maxHeight);

bool LoadShape(const std::string& filename, Math::Shape& shape, Math::BoundingBox& bb);
bool SaveShape(const std::string& filename, const Math::Shape& shape);
bool SaveShapeToOBJ(const std::string& filename, const Math::Shape& shape);
bool LoadUrhoModel(const std::string& filename, Math::Shape& shape);
bool LoadShapeFromOBJ(const std::string& filename, Math::Shape& shape);

}
