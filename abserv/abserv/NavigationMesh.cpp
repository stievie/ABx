#include "stdafx.h"
#include "NavigationMesh.h"

namespace Game {

NavigationMesh::NavigationMesh() :
    AssetImpl<NavigationMesh>(),
    navMesh_(nullptr),
    navQuery_(nullptr)
{
    navQuery_ = dtAllocNavMeshQuery();
}


NavigationMesh::~NavigationMesh()
{
    dtFreeNavMeshQuery(navQuery_);
    dtFreeNavMesh(navMesh_);
}

}
