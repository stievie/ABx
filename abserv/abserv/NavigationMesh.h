#pragma once

#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include "Asset.h"
#include "Vector3.h"

namespace Navigation {

struct FindPathData;

/// Navigation Mesh constructed from Map and Obstacles
class NavigationMesh : public IO::AssetImpl<NavigationMesh>
{
private:
    dtNavMesh* navMesh_;
    dtNavMeshQuery* navQuery_;
    std::unique_ptr<dtQueryFilter> queryFilter_;
    std::unique_ptr<FindPathData> pathData_;
    dtStatus pathFindState_;
public:
    NavigationMesh();
    ~NavigationMesh() override;

    static std::string GetStatusString(dtStatus status);

    void SetNavMesh(dtNavMesh* value)
    {
        if (navMesh_)
            dtFreeNavMesh(navMesh_);

        navMesh_ = value;
        navQuery_->init(navMesh_, 2048);
    }

    dtNavMesh* GetNavMesh() const { return navMesh_; }
    dtNavMeshQuery* GetNavQuery() const { return navQuery_; }

    /// Find a path between world space points. Return non-empty list of points if successful.
    /// Extents specifies how far off the navigation mesh the points can be.
    bool FindPath(std::vector<Math::Vector3>& dest, const Math::Vector3& start, const Math::Vector3& end,
        const Math::Vector3& extends = Math::Vector3::One, const dtQueryFilter* filter = nullptr);

    /// Find the nearest point on the navigation mesh to a given point. Extents specifies how far out from the specified point to check along each axis.
    Math::Vector3 FindNearestPoint(const Math::Vector3& point, const Math::Vector3& extents,
        const dtQueryFilter* filter = nullptr, dtPolyRef* nearestRef = nullptr);
};

}
