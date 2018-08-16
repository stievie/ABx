#include "stdafx.h"
#include "NavigationMesh.h"
#include "Profiler.h"

namespace Navigation {

static const int MAX_POLYS = 2048;

/// Temporary data for finding a path.
struct FindPathData
{
    // Polygons.
    dtPolyRef polys_[MAX_POLYS];
    // Polygons on the path.
    dtPolyRef pathPolys_[MAX_POLYS];
    // Points on the path.
    Math::Vector3 pathPoints_[MAX_POLYS];
    // Flags on the path.
    unsigned char pathFlags_[MAX_POLYS];
};

NavigationMesh::NavigationMesh() :
    AssetImpl<NavigationMesh>(),
    navMesh_(nullptr),
    navQuery_(dtAllocNavMeshQuery()),
    queryFilter_(std::make_unique<dtQueryFilter>()),
    pathData_(std::make_unique<FindPathData>())
{
}


NavigationMesh::~NavigationMesh()
{
    dtFreeNavMeshQuery(navQuery_);
    if (navMesh_)
        dtFreeNavMesh(navMesh_);
}

bool NavigationMesh::FindPath(std::vector<Math::Vector3>& dest,
    const Math::Vector3& start, const Math::Vector3& end,
    const Math::Vector3& extends /* = Math::Vector3::One */,
    const dtQueryFilter* filter /* = nullptr */)
{
    dest.clear();

    AB_PROFILE;

    const dtQueryFilter* queryFilter = filter ? filter : queryFilter_.get();
    dtPolyRef startRef = 0;
    dtPolyRef endRef = 0;
    dtStatus startStatus = navQuery_->findNearestPoly(start.Data(), extends.Data(), queryFilter, &startRef, nullptr);
    if (dtStatusFailed(startStatus))
    {
#ifdef DEBUG_NAVIGATION
        LOG_WARNING << "findNearestPoly() Failed with startStatus " << startStatus;
        if (dtStatusDetail(startStatus, DT_WRONG_MAGIC))
            LOG_WARNING << " DT_WRONG_MAGIC";
        if (dtStatusDetail(startStatus, DT_WRONG_VERSION))
            LOG_WARNING << " DT_WRONG_VERSION";
        if (dtStatusDetail(startStatus, DT_OUT_OF_MEMORY))
            LOG_WARNING << " DT_OUT_OF_MEMORY";
        if (dtStatusDetail(startStatus, DT_INVALID_PARAM))
            LOG_WARNING << " DT_INVALID_PARAM";
        if (dtStatusDetail(startStatus, DT_BUFFER_TOO_SMALL))
            LOG_WARNING << " DT_BUFFER_TOO_SMALL";
        if (dtStatusDetail(startStatus, DT_OUT_OF_NODES))
            LOG_WARNING << " DT_OUT_OF_NODES";
        if (dtStatusDetail(startStatus, DT_PARTIAL_RESULT))
            LOG_WARNING << " DT_PARTIAL_RESULT";
        LOG_WARNING << std::endl;
#endif
        return false;
    }

    dtStatus endStatus = navQuery_->findNearestPoly(end.Data(), extends.Data(), queryFilter, &endRef, nullptr);
    if (dtStatusFailed(endStatus))
    {
#ifdef DEBUG_NAVIGATION
        LOG_WARNING << "findNearestPoly() Failed with endStatus " << endStatus;
        if (dtStatusDetail(endStatus, DT_WRONG_MAGIC))
            LOG_WARNING << " DT_WRONG_MAGIC";
        if (dtStatusDetail(endStatus, DT_WRONG_VERSION))
            LOG_WARNING << " DT_WRONG_VERSION";
        if (dtStatusDetail(endStatus, DT_OUT_OF_MEMORY))
            LOG_WARNING << " DT_OUT_OF_MEMORY";
        if (dtStatusDetail(endStatus, DT_INVALID_PARAM))
            LOG_WARNING << " DT_INVALID_PARAM";
        if (dtStatusDetail(endStatus, DT_BUFFER_TOO_SMALL))
            LOG_WARNING << " DT_BUFFER_TOO_SMALL";
        if (dtStatusDetail(endStatus, DT_OUT_OF_NODES))
            LOG_WARNING << " DT_OUT_OF_NODES";
        if (dtStatusDetail(endStatus, DT_PARTIAL_RESULT))
            LOG_WARNING << " DT_PARTIAL_RESULT";
        LOG_WARNING << std::endl;
#endif
        return false;
    }

#ifdef DEBUG_NAVIGATION
    LOG_DEBUG << "startRef " << startRef << ", endRef " << endRef << std::endl;
#endif
    if (!startRef || !endRef)
        return false;

    int numPolys = 0;
    int numPathPoints = 0;

    dtStatus findStatus = navQuery_->findPath(startRef, endRef, &start.x_, &end.x_, queryFilter,
        pathData_->polys_, &numPolys, MAX_POLYS);
    if (dtStatusFailed(findStatus) || !numPolys)
        return false;

    Math::Vector3 actualEnd = end;

    // If full path was not found, clamp end point to the end polygon
    if (pathData_->polys_[numPolys - 1] != endRef)
        navQuery_->closestPointOnPoly(pathData_->polys_[numPolys - 1], &end.x_, &actualEnd.x_, nullptr);

    navQuery_->findStraightPath(&start.x_, &actualEnd.x_, pathData_->polys_, numPolys,
        &pathData_->pathPoints_[0].x_, pathData_->pathFlags_, pathData_->pathPolys_, &numPathPoints, MAX_POLYS);

    // First point is start point skip it
    for (int i = 0; i < numPathPoints; ++i)
    {
        dest.push_back(pathData_->pathPoints_[i]);
    }

    return true;
}

}
