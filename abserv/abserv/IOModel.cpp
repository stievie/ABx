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

#include "IOModel.h"
#include <absmath/Shape.h>
#include <eastl.hpp>

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
    asset.SetShape(ea::make_unique<Math::Shape>());
    Math::Shape* modelShape = asset.GetShape();

    input.read(reinterpret_cast<char*>(&modelShape->vertexCount_), sizeof(modelShape->vertexCount_));
    modelShape->vertexData_.resize(modelShape->vertexCount_);
    for (unsigned i = 0; i < modelShape->vertexCount_; ++i)
        input.read((char*)modelShape->vertexData_[i].Data(), sizeof(float) * 3);

    input.read(reinterpret_cast<char*>(&modelShape->indexCount_), sizeof(modelShape->indexCount_));
    modelShape->indexData_.resize(modelShape->indexCount_);
    input.read((char*)modelShape->indexData_.data(), sizeof(unsigned) * modelShape->indexCount_);

    return true;
}

}
