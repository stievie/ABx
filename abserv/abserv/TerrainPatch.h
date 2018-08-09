#pragma once

#include "GameObject.h"
#include "Point.h"
#include "BoundingBox.h"

namespace Game {

class Terrain;

class TerrainPatch : public GameObject
{
private:
    Terrain* owner_;
    Math::BoundingBox boundingBox_;
public:
    TerrainPatch(Terrain* owner, const Math::Point<int>& offset,
        const Math::Point<int>& size);
    ~TerrainPatch();

    /// Process octree raycast. May be called from a worker thread.
    void ProcessRayQuery(const Math::RayOctreeQuery& query, std::vector<Math::RayQueryResult>& results) override;
    Math::BoundingBox GetWorldBoundingBox() const override
    {
        return boundingBox_.Transformed(transformation_.GetMatrix());
    }
    Math::BoundingBox GetBoundingBox() const override
    {
        return boundingBox_;
    }
    AB::GameProtocol::GameObjectType GetType() const override
    {
        return AB::GameProtocol::ObjectTypeTerrainPatch;
    }

    Math::Point<int> offset_;
    Math::Point<int> size_;
};

}
