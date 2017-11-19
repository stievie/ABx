#pragma once

#include <DetourNavMesh.h>
#include "Asset.h"

namespace Game {

/// Navigation Mesh constructed from Map and Obstacles
class NavigationMesh : public IO::AssetImpl<NavigationMesh>
{
private:
    dtNavMesh* navMesh_;
public:
    NavigationMesh();
    virtual ~NavigationMesh();

    void SetNavMesh(dtNavMesh* value)
    {
        if (navMesh_)
            dtFreeNavMesh(navMesh_);

        navMesh_ = value;
    }
};

}
