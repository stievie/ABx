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

#include "Asset.h"
#include <absmath/Vector3.h>
#include <absmath/HeightMap.h>
#include <absmath/Transformation.h>
#include <absmath/Point.h>
#include <eastl.hpp>

namespace Game {

inline constexpr int DEFAULT_PATCH_SIZE = 32;
inline constexpr int MIN_PATCH_SIZE = 4;
inline constexpr int MAX_PATCH_SIZE = 128;

class HeightMap final : public IO::Asset, public Math::HeightMap
{

};

class Terrain final : public IO::Asset
{
private:
    // Must be shared_ptr CollisionShape may also own it
    ea::shared_ptr<HeightMap> heightMap_;
    ea::shared_ptr<HeightMap> heightMap2_;
    mutable bool matrixDirty_{ true };
public:
    Terrain();
    ~Terrain() override;

    void SetHeightMap(ea::shared_ptr<HeightMap> val)
    {
        heightMap_ = val;
    }
    void SetHeightMap2(ea::shared_ptr<HeightMap> val)
    {
        heightMap2_ = val;
    }
    Math::HeightMap* GetHeightMap() const
    {
        if (heightMap_)
            return heightMap_.get();
        return nullptr;
    }
    Math::HeightMap* GetHeightMap2() const
    {
        if (heightMap2_)
            return heightMap2_.get();
        return nullptr;
    }
    void Initialize();
    void SetSpacing(const Math::Vector3& value);
    float GetHeight(const Math::Vector3& world) const;

    Math::Transformation transformation_;
    int patchSize_{ DEFAULT_PATCH_SIZE };
    Math::Point<int> numPatches_;
};

}
