#pragma once

#include "Asset.h"
#include "Vector3.h"
#include "HeightMap.h"
#include "Shape.h"
#include "Transformation.h"
#include "CollisionShape.h"
#include "Point.h"

namespace Game {

static constexpr int DEFAULT_PATCH_SIZE = 32;
static constexpr int MIN_PATCH_SIZE = 4;
static constexpr int MAX_PATCH_SIZE = 128;

class TerrainPatch;

class Terrain : public IO::Asset
{
private:
    // Must be shared_ptr CollisionShape may also own it
    std::shared_ptr<Math::HeightMap> heightMap_;
public:
    Terrain();

    void SetHeightMap(std::shared_ptr<Math::HeightMap> val)
    {
        heightMap_ = val;
    }
    Math::HeightMap* GetHeightMap() const
    {
        if (heightMap_)
            return heightMap_.get();
        return nullptr;
    }
    float GetHeight(const Math::Vector3& world) const
    {
        if (!heightMap_)
            return 0.0f;
        heightMap_->matrix_ = transformation_.GetMatrix();
        return heightMap_->GetHeight(world);
    }

    Math::Transformation transformation_;
    int patchSize_{ DEFAULT_PATCH_SIZE };
    Math::Point<int> numPatches_;
};

}
