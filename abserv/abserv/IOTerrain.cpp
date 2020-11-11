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

#include "IOTerrain.h"
#include <fstream>
#include <sa/StringTempl.h>

namespace IO {

bool IOTerrain::Import(Game::Terrain& asset, const std::string& name)
{
    std::fstream input(name, std::ios::binary | std::fstream::in);
    if (!input.is_open())
        return false;

    //    AB_PROFILE;

    char sig[4];
    input.read(sig, 4);
    if (sig[0] != 'H' || sig[1] != 'M' || sig[2] != '\0' || sig[3] != '\0')
        return false;

    ea::shared_ptr<Math::HeightMap> heightMap = ea::make_shared<Math::HeightMap>();
    asset.SetHeightMap(heightMap);

    input.read((char*)&heightMap->numVertices_.x_, sizeof(int32_t));
    input.read((char*)&heightMap->numVertices_.y_, sizeof(int32_t));

    input.read((char*)&heightMap->patchSize_, sizeof(int32_t));

    input.read((char*)&heightMap->patchWorldSize_.x_, sizeof(float));
    input.read((char*)&heightMap->patchWorldSize_.y_, sizeof(float));
    input.read((char*)&heightMap->numPatches_.x_, sizeof(int32_t));
    input.read((char*)&heightMap->numPatches_.y_, sizeof(int32_t));
    input.read((char*)&heightMap->patchWorldOrigin_.x_, sizeof(float));
    input.read((char*)&heightMap->patchWorldOrigin_.y_, sizeof(float));

    input.read((char*)&heightMap->minHeight_, sizeof(float));
    input.read((char*)&heightMap->maxHeight_, sizeof(float));

#ifdef _DEBUG
    //    LOG_DEBUG << "nX=" << heightMap->numVertices_.x_ << " nY=" << heightMap->numVertices_.y_ <<
    //        " minHeight=" << heightMap->minHeight_ <<
    //        " maxHeight=" << heightMap->maxHeight_ << std::endl;
#endif
    uint32_t heightsCount;
    input.read((char*)&heightsCount, sizeof(uint32_t));
    heightMap->heightData_.resize((size_t)heightsCount);
    input.read((char*)heightMap->heightData_.data(), sizeof(float) * (size_t)heightsCount);
    input.close();

    return true;
}

}
