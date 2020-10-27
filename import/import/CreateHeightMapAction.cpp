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

#include "stdafx.h"
#include "CreateHeightMapAction.h"
#include "ObjWriter.h"
#include <fstream>
#include <absmath/MathUtils.h>
#include <absmath/VectorMath.h>
#include <limits>
#include <abscommon/StringUtils.h>
#include <sa/StringTempl.h>
#include <absmath/Point.h>

void CreateHeightMapAction::SaveHeightMap()
{
    std::string fileName;
    if (!outputDirectory_.empty())
        fileName = Utils::ConcatPath(outputDirectory_, sa::ExtractFileName<char>(file_) + ".hm");
    else
        fileName = file_ + ".hm";

    std::fstream output(fileName, std::ios::binary | std::fstream::out);
    output.write((char*)"HM\0\0", 4);
    // Height Data
    output.write((char*)&width_, sizeof(width_));
    output.write((char*)&height_, sizeof(height_));

    output.write((char*)&minHeight_, sizeof(minHeight_));
    output.write((char*)&maxHeight_, sizeof(maxHeight_));
    unsigned c = (unsigned)heightData_.size();
    output.write((char*)&c, sizeof(c));
    output.write((char*)heightData_.data(), c * sizeof(float));

    output.close();
    std::cout << "Created " << fileName << std::endl;
}

void CreateHeightMapAction::CreateGeometry()
{
//    Math::Point<float> patchWorldSize = { spacing_.x_ * (float)patchSize_, spacing_.z_ * (float)patchSize_ };
    Math::Point<int> numPatches = { (width_ - 1) / patchSize_, (height_ - 1) / patchSize_ };
    Math::Point<int> numVertices = { numPatches.x_ * patchSize_ + 1, numPatches.y_ * patchSize_ + 1 };

    unsigned imgRow = width_ * components_;

    auto getHeight = [&](int x, int z, bool rightHand = false) -> float
    {
        if (!data_)
            return 0.0f;

        // From bottom to top
        int offset;
        if (rightHand)
            offset = imgRow * (numVertices.y_ - 1 - z) + ((numVertices.x_ - 1) - x);
        else
            offset = imgRow * (numVertices.y_ - 1 - z) + x;

        if (components_ == 1)
            return (float)data_[offset];

        // If more than 1 component, use the green channel for more accuracy
        return (float)data_[offset] +
            (float)data_[offset + 1] / 256.0f;
    };

    auto getNormal = [&](int x, int z) -> Math::Vector3
    {
        float baseHeight = getHeight(x, z);
        float nSlope = getHeight(x, z - 1) - baseHeight;
        float neSlope = getHeight(x + 1, z - 1) - baseHeight;
        float eSlope = getHeight(x + 1, z) - baseHeight;
        float seSlope = getHeight(x + 1, z + 1) - baseHeight;
        float sSlope = getHeight(x, z + 1) - baseHeight;
        float swSlope = getHeight(x - 1, z + 1) - baseHeight;
        float wSlope = getHeight(x - 1, z) - baseHeight;
        float nwSlope = getHeight(x - 1, z - 1) - baseHeight;
        float up = 0.5f * (spacing_.x_ + spacing_.z_);

        using namespace Math;
        return (Vector3(0.0f, up, nSlope) +
            Vector3(-neSlope, up, neSlope) +
            Vector3(-eSlope, up, 0.0f) +
            Vector3(-seSlope, up, -seSlope) +
            Vector3(0.0f, up, -sSlope) +
            Vector3(swSlope, up, -swSlope) +
            Vector3(wSlope, up, 0.0f) +
            Vector3(nwSlope, up, nwSlope)).Normal();
    };

    minHeight_ = std::numeric_limits<float>::max();
    maxHeight_ = std::numeric_limits<float>::lowest();

    heightData_.resize((size_t)numVertices.x_ * (size_t)numVertices.y_);
    for (int y = 0; y < numVertices.y_; ++y)
    {
        for (int x = 0; x < numVertices.x_; ++x)
        {
            float fy = getHeight(x, y);
            heightData_[y * numVertices.x_ + x] = fy;
            if (minHeight_ > fy)
                minHeight_ = fy;
            if (maxHeight_ < fy)
                maxHeight_ = fy;
        }
    }

}

void CreateHeightMapAction::Execute()
{
    data_ = stbi_load(file_.c_str(), &width_, &height_, &components_, 0);

    if (!data_)
    {
        std::cout << "Error loading file " << file_ << std::endl;
        return;
    }

    CreateGeometry();

    SaveHeightMap();
    free(data_);
    data_ = nullptr;
}
