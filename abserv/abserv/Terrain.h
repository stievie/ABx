#pragma once

#include "Asset.h"
#include "Vector3.h"
#include "HeightMap.h"

namespace Game {

class Terrain : public IO::AssetImpl<Terrain>
{
public:
    Terrain();
    virtual ~Terrain();

    std::unique_ptr<Math::HeightMap> heightMap_;
    std::vector<Math::Vector3> vertices_;
};

}
