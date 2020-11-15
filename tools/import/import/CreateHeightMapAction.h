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

#include <sa/Compiler.h>

PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_MSVC(4244 4456)
#include <stb_image.h>
PRAGMA_WARNING_POP
#include <absmath/Vector3.h>
#include <absmath/Point.h>
#include <eastl.hpp>

/// Creates Heightmap and Mesh from Image
class CreateHeightMapAction
{
private:
    std::string file_;
    std::string outputDirectory_;
    ea::vector<float> heightData_;
    float minHeight_;
    float maxHeight_;

    int width_{ 0 };
    int height_{ 0 };
    int components_{ 0 };
    stbi_uc* data_{ nullptr };
    Math::Point<int> numPatches_;
    Math::Point<int> numVertices_;
    Math::Point<float> patchWorldSize_;
    Math::Point<float> patchWorldOrigin_;
    void SaveHeightMap();
    void CreateGeometry();
public:
    CreateHeightMapAction(const std::string& file, const std::string& outDir) :
        file_(file),
        outputDirectory_(outDir),
        patchSize_(32)
    {}
    ~CreateHeightMapAction()
    {
        if (data_)
            free(data_);
    }
    void Execute();

    Math::Vector3 spacing_;
    int32_t patchSize_{ 32 };
};
