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

bool IOTerrain::LoadHeightMap(Game::Terrain& asset, const std::string& name)
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

    heightMap->spacing_ = spacing_;

    input.read((char*)&heightMap->numVertices_.x_, sizeof(int));
    input.read((char*)&heightMap->numVertices_.y_, sizeof(int));
    input.read((char*)&heightMap->minHeight_, sizeof(float));
    input.read((char*)&heightMap->maxHeight_, sizeof(float));

#ifdef _DEBUG
    //    LOG_DEBUG << "nX=" << heightMap->numVertices_.x_ << " nY=" << heightMap->numVertices_.y_ <<
    //        " minHeight=" << heightMap->minHeight_ <<
    //        " maxHeight=" << heightMap->maxHeight_ << std::endl;
#endif
    unsigned heightsCount;
    input.read((char*)&heightsCount, sizeof(unsigned));
    heightMap->heightData_.resize(heightsCount);
    input.read((char*)heightMap->heightData_.data(), sizeof(float) * heightsCount);

    heightMap->ProcessData();

    return true;
}

float IOTerrain::GetRawHeight(int x, int z) const
{
    if (!data_)
        return 0.0f;

    // From bottom to top
    int offset = (((height_ - 1) - z) * width_ + x) * components_;

    if (components_ == 1)
        return (float)data_[offset];

    // If more than 1 component, use the green channel for more accuracy
    return (float)data_[offset] +
        (float)data_[offset + 1] / 256.0f;
}

void IOTerrain::CreateGeometry(Math::HeightMap& hm)
{
    hm.minHeight_ = std::numeric_limits<float>::max();
    hm.maxHeight_ = std::numeric_limits<float>::lowest();
    hm.heightData_.resize(width_ * height_);
    for (int z = 0; z < height_; ++z)
    {
        for (int x = 0; x < width_; ++x)
        {
            float fy = GetRawHeight(x, z);
            hm.heightData_[z * width_ + x] = fy;
            if (hm.minHeight_ > fy)
                hm.minHeight_ = fy;
            if (hm.maxHeight_ < fy)
                hm.maxHeight_ = fy;
        }
    }
}

bool IOTerrain::LoadPNG(Game::Terrain& asset, const std::string& name)
{
    AB_PROFILE;
    data_ = stbi_load(name.c_str(), &width_, &height_, &components_, 0);
    if (!data_)
    {
        LOG_ERROR << "Unable to load file " << name << std::endl;
        return false;
    }
    ea::shared_ptr<Math::HeightMap> heightMap = ea::make_shared<Math::HeightMap>();
    asset.SetHeightMap(heightMap);

    heightMap->spacing_ = spacing_;
    heightMap->numVertices_ = { width_, height_ };
    CreateGeometry(*heightMap);
    heightMap->ProcessData();
    return true;
}

bool IOTerrain::Import(Game::Terrain& asset, const std::string& name)
{
    const std::string ext = sa::GetFileExt<char>(name);
    if (ext == ".hm" || ext == ".terrain")
        return LoadHeightMap(asset, name);
    if (ext == ".png")
        return LoadPNG(asset, name);
    LOG_ERROR << "Unknown height map file " << name << std::endl;
    return false;
}

}
