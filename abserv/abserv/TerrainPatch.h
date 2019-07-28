#pragma once

#include "GameObject.h"
#include "Point.h"
#include "BoundingBox.h"

namespace Game {

class Terrain;

class TerrainPatch : public GameObject
{
private:
    std::weak_ptr<Terrain> owner_;
    Math::BoundingBox boundingBox_;
    float CastRay(const Math::Vector3& origin, const Math::Vector3& direction, float maxDist) const;
public:
    TerrainPatch(std::shared_ptr<Terrain> owner,
        const Math::Point<int>& offset,
        const Math::Point<int>& size);
    ~TerrainPatch() override = default;

    /// Process octree raycast. May be called from a worker thread.
    void ProcessRayQuery(const Math::RayOctreeQuery& query, std::vector<Math::RayQueryResult>& results) override;
    Math::BoundingBox GetWorldBoundingBox() const override;
    Math::BoundingBox GetBoundingBox() const override
    {
        return boundingBox_;
    }
    AB::GameProtocol::GameObjectType GetType() const override
    {
        return AB::GameProtocol::ObjectTypeTerrainPatch;
    }
    float GetHeight(const Math::Vector3& position) const;

    Math::Point<int> offset_;
    Math::Point<int> size_;
};

}
