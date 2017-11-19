#include "stdafx.h"
#include "NavigationMesh.h"

namespace Game {

NavigationMesh::NavigationMesh() :
    AssetImpl<NavigationMesh>(),
    navMesh_(nullptr)
{
}


NavigationMesh::~NavigationMesh()
{
    dtFreeNavMesh(navMesh_);
}

}
