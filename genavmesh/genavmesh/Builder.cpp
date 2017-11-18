#include "Builder.h"

Builder::~Builder()
{
    dtFreeNavMeshQuery(m_navQuery);
    dtFreeNavMesh(m_navMesh);
}
