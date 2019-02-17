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

    std::shared_ptr<Math::HeightMap> heightMap = std::make_shared<Math::HeightMap>();
    asset->SetHeightMap(heightMap);

    // Urho3D default spacing
    heightMap->spacing_ = Math::Vector3(1.0f, 0.25f, 1.0f);

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

}
