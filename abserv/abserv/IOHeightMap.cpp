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

#include "IOHeightMap.h"
#include <fstream>
#include <sa/StringTempl.h>

namespace IO {

bool IOHeightMap::Import(Game::HeightMap& asset, const std::string& name)
{
    std::fstream input(name, std::ios::binary | std::fstream::in);
    if (!input.is_open())
        return false;

    char sig[4];
    input.read(sig, 4);
    if (sig[0] != 'H' || sig[1] != 'M' || sig[2] != '\0' || sig[3] != '\0')
        return false;

    input.read((char*)&asset.numVertices_.x_, sizeof(int32_t));
    input.read((char*)&asset.numVertices_.y_, sizeof(int32_t));

    input.read((char*)&asset.patchSize_, sizeof(int32_t));

    input.read((char*)&asset.patchWorldSize_.x_, sizeof(float));
    input.read((char*)&asset.patchWorldSize_.y_, sizeof(float));
    input.read((char*)&asset.numPatches_.x_, sizeof(int32_t));
    input.read((char*)&asset.numPatches_.y_, sizeof(int32_t));
    input.read((char*)&asset.patchWorldOrigin_.x_, sizeof(float));
    input.read((char*)&asset.patchWorldOrigin_.y_, sizeof(float));

    input.read((char*)&asset.minHeight_, sizeof(float));
    input.read((char*)&asset.maxHeight_, sizeof(float));

#ifdef _DEBUG
    LOG_DEBUG << name << ": " << "nX=" << asset.numVertices_.x_ << " nY=" << asset.numVertices_.y_ <<
        " minHeight=" << asset.minHeight_ <<
        " maxHeight=" << asset.maxHeight_ << std::endl;
#endif
    uint32_t heightsCount;
    input.read((char*)&heightsCount, sizeof(uint32_t));
    asset.heightData_.resize((size_t)heightsCount);
    input.read((char*)asset.heightData_.data(), sizeof(float) * (size_t)heightsCount);
    input.close();

    return true;
}

}
