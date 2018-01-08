#include "stdafx.h"
#include "Map.h"
#include "DataProvider.h"

namespace Game {

Map::Map() :
    navMesh_(nullptr)
{
    octree_ = std::make_unique<Math::Octree>();
}


Map::~Map()
{
}

bool Map::Load()
{
    navMesh_ = IO::DataProvider::Instance.GetAsset<NavigationMesh>(data_.directory + "/navmesh.bin");
    if (!navMesh_)
        return false;
//    terrain_ = IO::DataProvider::Instance.GetAsset<Terrain>(data_.directory + "/terrain.obj");
//    if (!terrain_)
//        return false;

    return true;
}

void Map::Update(uint32_t delta)
{
    AB_UNUSED(delta);
    octree_->Update();
}

}
