#include "stdafx.h"
#include "Terrain.h"
#include "TerrainPatch.h"

namespace Game {

Terrain::Terrain() :
    heightMap_(nullptr),
    patchSize_(DEFAULT_PATCH_SIZE)
{
}

Terrain::~Terrain()
{
}

void Terrain::CreatePatches()
{
    patches_.clear();
    numPatches_.x_ = (heightMap_->GetWidth() - 1) / patchSize_;
    numPatches_.y_ = (heightMap_->GetHeight() - 1) / patchSize_;
    patches_.reserve(numPatches_.x_ * numPatches_.y_);
    for (int y = 0; y < numPatches_.y_; ++y)
    {
        for (int x = 0; x < numPatches_.x_; ++x)
        {
            patches_.push_back(
                std::make_unique<TerrainPatch>(this, Math::Point<int>(x, y),
                    Math::Point<int>(patchSize_, patchSize_))
            );
        }
    }
}

TerrainPatch* Terrain::GetPatch(unsigned index) const
{
    return index < patches_.size() ? patches_[index].get() : nullptr;
}

TerrainPatch* Terrain::GetPatch(int x, int z) const
{
    if (x < 0 || x >= numPatches_.x_ || z < 0 || z >= numPatches_.y_)
        return nullptr;
    else
        return GetPatch((unsigned)(z * numPatches_.x_ + x));
}

}