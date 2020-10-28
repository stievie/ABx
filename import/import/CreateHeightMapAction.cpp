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
    output.write((char*)&numVertices_.x_, sizeof(int32_t));
    output.write((char*)&numVertices_.y_, sizeof(int32_t));

    output.write((char*)&patchSize_, sizeof(int32_t));

    output.write((char*)&patchWorldSize_.x_, sizeof(float));
    output.write((char*)&patchWorldSize_.y_, sizeof(float));
    output.write((char*)&numPatches_.x_, sizeof(int32_t));
    output.write((char*)&numPatches_.y_, sizeof(int32_t));
    output.write((char*)&patchWorldOrigin_.x_, sizeof(float));
    output.write((char*)&patchWorldOrigin_.y_, sizeof(float));

    output.write((char*)&minHeight_, sizeof(float));
    output.write((char*)&maxHeight_, sizeof(float));

    uint32_t c = (uint32_t)heightData_.size();
    output.write((char*)&c, sizeof(uint32_t));
    output.write((char*)heightData_.data(), (size_t)c * sizeof(float));

    output.close();
    std::cout << "Created " << fileName << std::endl;
}

void CreateHeightMapAction::CreateGeometry()
{
    patchWorldSize_ = { spacing_.x_ * (float)patchSize_, spacing_.z_ * (float)patchSize_ };
    numPatches_ = { (width_ - 1) / patchSize_, (height_ - 1) / patchSize_ };
    numVertices_ = { numPatches_.x_ * patchSize_ + 1, numPatches_.y_ * patchSize_ + 1 };
    patchWorldOrigin_ = { -0.5f * (float)numPatches_.x_ * patchWorldSize_.x_, -0.5f * (float)numPatches_.y_ * patchWorldSize_.y_ };

    int imgRow = width_ * components_;
    auto getHeight = [&](int x, int z) -> float
    {
        if (!data_)
            return 0.0f;

        // From bottom to top
        int offset = imgRow * (numVertices_.y_ - 1 - z) + components_ * x;

        if (components_ == 1)
            return (float)data_[offset];

        // If more than 1 component, use the green channel for more accuracy
        return (float)data_[offset] +
            (float)data_[offset + 1] / 256.0f;
    };

    minHeight_ = std::numeric_limits<float>::max();
    maxHeight_ = std::numeric_limits<float>::lowest();

    heightData_.resize(numVertices_.x_ * numVertices_.y_);
    for (int y = 0; y < numVertices_.y_; ++y)
    {
        for (int x = 0; x < numVertices_.x_; ++x)
        {
            float fy = getHeight(x, y) * spacing_.y_;
            heightData_[y * numVertices_.x_ + x] = fy;
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
