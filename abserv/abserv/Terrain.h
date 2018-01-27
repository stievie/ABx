#pragma once

#include "Asset.h"
#include "Vector3.h"
#include "HeightMap.h"
#include "Shape.h"
#include "Transformation.h"

namespace Game {

class Terrain : public IO::AssetImpl<Terrain>
{
public:
    Terrain();
    virtual ~Terrain();

    Math::Transformation transformation_;
    std::unique_ptr<Math::HeightMap> heightMap_;
    std::unique_ptr<Math::Shape> shape_;
    float GetHeight(const Math::Vector3& world) const
    {
        return heightMap_->GetHeight(world, transformation_.GetMatrix());
    }
};

}
