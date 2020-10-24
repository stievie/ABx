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

#pragma once

#include "IOAsset.h"
#include "Terrain.h"
#include <stb/stb_image.h>

namespace IO {

class IOTerrain : public IOAssetImpl<Game::Terrain>
{
private:
    int width_{ 0 };
    int height_{ 0 };
    int components_{ 0 };
    stbi_uc* data_{ nullptr };
    // Urho3D default spacing
    Math::Vector3 spacing_{ 1.0f, 0.25f, 1.0f };
    void CreateGeometry(Math::HeightMap& hm);
    float GetRawHeight(int x, int z) const;
    Math::Vector3 GetRawNormal(int x, int z) const;
    bool LoadHeightMap(Game::Terrain& asset, const std::string& name);
    bool LoadPNG(Game::Terrain& asset, const std::string& name);
public:
    bool Import(Game::Terrain& asset, const std::string& name) override;
};

}
