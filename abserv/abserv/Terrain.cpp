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

#include "Terrain.h"
#include "TerrainPatch.h"

namespace Game {

void HeightMap::OnChanged()
{
    ProcessData();
}

Terrain::Terrain() :
    IO::Asset()
{
}

Terrain::~Terrain() = default;

void Terrain::SetSpacing(const Math::Vector3& value)
{
    if (heightMap_)
        heightMap_->spacing_ = value;
    if (heightMap2_)
        heightMap2_->spacing_ = value;
}

void Terrain::Initialize()
{
    if (heightMap_)
        heightMap_->ProcessData();
    if (heightMap2_)
        heightMap2_->ProcessData();
}

float Terrain::GetHeight(const Math::Vector3& world) const
{
    if (!heightMap_)
        return 0.0f;
    if (matrixDirty_)
    {
        const Math::Matrix4 matrix = transformation_.GetMatrix();
        heightMap_->SetMatrix(matrix);
        if (heightMap2_)
            heightMap2_->SetMatrix(matrix);
        matrixDirty_ = false;
    }

    float result = heightMap_->GetHeight(world);
    if (!heightMap2_)
        return result;

    float result2 = heightMap2_->GetHeight(world);
//    LOG_DEBUG << "height1 " << result << " height2 " << result2 << std::endl;
    // Layer2 must be above layer1
    if (Math::IsNegInfinite(result2) || result2 < result)
        return result;

    // If the difference is smaller than the height of the character it can't be the lower height
    float diff12 = result2 - result;
    if (diff12 < 1.7f)
        return result2;

    // Otherwise use the closer value to the current height
    if (fabs(world.y_ - result) < fabs(world.y_ - result2))
        return result;
    return result2;
}

}
