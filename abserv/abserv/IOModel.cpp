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

#include "stdafx.h"
#include "IOModel.h"
#include "Profiler.h"
#include "Shape.h"

namespace IO {

bool IOModel::Import(Game::Model& asset, const std::string& name)
{
    std::fstream input(name, std::ios::binary | std::fstream::in);
    if (!input.is_open())
        return false;

    AB_PROFILE;

    char sig[4];
    input.read(sig, 4);
    if (sig[0] != 'M' || sig[1] != 'O' || sig[2] != 'D' || sig[3] != 'L')
        return false;

    // Read bounding box
    input.read((char*)asset.boundingBox_.min_.Data(), sizeof(float) * 3);
    input.read((char*)asset.boundingBox_.max_.Data(), sizeof(float) * 3);

    // Read shape data
    asset.shape_ = std::make_unique<Math::Shape>();

    input.read(reinterpret_cast<char*>(&asset.shape_->vertexCount_), sizeof(asset.shape_->vertexCount_));
    asset.shape_->vertexData_.resize(asset.shape_->vertexCount_);
    for (unsigned i = 0; i < asset.shape_->vertexCount_; ++i)
    {
        input.read((char*)asset.shape_->vertexData_[i].Data(), sizeof(float) * 3);
    }

    input.read(reinterpret_cast<char*>(&asset.shape_->indexCount_), sizeof(asset.shape_->indexCount_));
    asset.shape_->indexData_.resize(asset.shape_->indexCount_);
    input.read((char*)asset.shape_->indexData_.data(), sizeof(unsigned) * asset.shape_->indexCount_);

    return true;
}

}
