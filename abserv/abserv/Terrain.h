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

class Terrain : public IO::AssetImpl<Terrain>
{
private:
    std::unique_ptr<Math::CollisionShape> collisionShape_;
    // TerrainPatches are also owned by the game
    std::vector<std::shared_ptr<TerrainPatch>> patches_;
    // Must be shared_ptr CollisionShape may also own it
    std::shared_ptr<Math::HeightMap> heightMap_;
public:
    Terrain();
    virtual ~Terrain();

    void SetHeightMap(std::shared_ptr<Math::HeightMap> val)
    {
        heightMap_ = val;
    }
    Math::HeightMap* GetHeightMap() const
    {
        return heightMap_.get();
    }
    float GetHeight(const Math::Vector3& world) const
    {
        heightMap_->matrix_ = transformation_.GetMatrix();
        return heightMap_->GetHeight(world);
    }
    void SetCollisionShape(std::unique_ptr<Math::CollisionShape> shape)
    {
        collisionShape_ = std::move(shape);
    }
    Math::CollisionShape* GetCollisionShape() const
    {
        if (!collisionShape_)
            return nullptr;
        return collisionShape_.get();
    }
    void CreatePatches();
    /// Return patch by index.
    TerrainPatch* GetPatch(unsigned index) const;
    /// Return patch by patch coordinates.
    TerrainPatch* GetPatch(int x, int z) const;
    size_t GetPatchesCount() const
    {
        return patches_.size();
    }

    Math::Transformation transformation_;
    int patchSize_;
    Math::Point<int> numPatches_;
};

}
