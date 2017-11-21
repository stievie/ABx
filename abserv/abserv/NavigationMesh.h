#pragma once

#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include "Asset.h"

namespace Game {

/// Navigation Mesh constructed from Map and Obstacles
class NavigationMesh : public IO::AssetImpl<NavigationMesh>
{
private:
    dtNavMesh* navMesh_;
    dtNavMeshQuery* navQuery_;
public:
    NavigationMesh();
    virtual ~NavigationMesh();

    void SetNavMesh(dtNavMesh* value)
    {
        if (navMesh_)
            dtFreeNavMesh(navMesh_);

        navMesh_ = value;
        navQuery_->init(navMesh_, 2048);
    }

    dtNavMesh* GetNavMesh() const { return navMesh_; }
    dtNavMeshQuery* GetNavQuery() const { return navQuery_; }
};

}
