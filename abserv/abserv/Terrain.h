#pragma once

#include "Asset.h"
#include "Vector3.h"
#include "ConvexHull.h"

namespace Game {

class Terrain : public IO::AssetImpl<Terrain>
{
private:
    std::unique_ptr<Math::ConvexHull> hullShape_;
public:
    Terrain();
    virtual ~Terrain();

    void BuildShape();
    std::vector<Math::Vector3> vertices_;
};

}
