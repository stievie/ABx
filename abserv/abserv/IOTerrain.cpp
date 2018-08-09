#include "stdafx.h"
#include "IOTerrain.h"
#include <fstream>
#include "HeightMap.h"
#include "Shape.h"
#include "Profiler.h"

namespace IO {

bool IOTerrain::Import(Game::Terrain* asset, const std::string& name)
{
    std::fstream input(name, std::ios::binary | std::fstream::in);
    if (!input.is_open())
        return false;

    AB_PROFILE;

    char sig[4];
    input.read(sig, 4);
    if (sig[0] != 'H' || sig[1] != 'M' || sig[2] != '\0' || sig[3] != '\0')
        return false;

    asset->heightMap_ = std::make_shared<Math::HeightMap>();

    // Urho3D default spacing
    asset->heightMap_->spacing_ = Math::Vector3(1.0f, 0.25f, 1.0f);

    input.read((char*)&asset->heightMap_->numVertices_.x_, sizeof(int));
    input.read((char*)&asset->heightMap_->numVertices_.y_, sizeof(int));
    input.read((char*)&asset->heightMap_->minHeight_, sizeof(float));
    input.read((char*)&asset->heightMap_->maxHeight_, sizeof(float));

#ifdef _DEBUG
    LOG_DEBUG << "nX=" << asset->heightMap_->numVertices_.x_ << " nY=" << asset->heightMap_->numVertices_.y_ <<
        " minHeight=" << asset->heightMap_->minHeight_ <<
        " maxHeight=" << asset->heightMap_->maxHeight_ << std::endl;
#endif
    unsigned heightsCount;
    input.read((char*)&heightsCount, sizeof(unsigned));
    asset->heightMap_->heightData_.resize(heightsCount);
    input.read((char*)asset->heightMap_->heightData_.data(), sizeof(float) * heightsCount);

    asset->heightMap_->ProcessData();
    asset->CreatePatches();

    asset->SetCollisionShape(
        std::make_unique<Math::CollisionShapeImpl<Math::HeightMap>>(Math::ShapeTypeHeightMap, asset->heightMap_)
    );

    return true;
}

}
