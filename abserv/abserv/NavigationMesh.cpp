/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include "NavigationMesh.h"
#include <abscommon/Random.h>
#include <abscommon/Subsystems.h>

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
    IO::Asset(),
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

void NavigationMesh::SetNavMesh(dtNavMesh* value)
{
    if (navMesh_)
        dtFreeNavMesh(navMesh_);

    navMesh_ = value;
    navQuery_->init(navMesh_, MAX_POLYS);
}

bool NavigationMesh::FindPath(std::vector<Math::Vector3>& dest,
    const Math::Vector3& start, const Math::Vector3& end,
    const Math::Vector3& extends /* = Math::Vector3::One */,
    const dtQueryFilter* filter /* = nullptr */)
{
    dest.clear();

//    AB_PROFILE;

    const dtQueryFilter* queryFilter = filter ? filter : queryFilter_.get();
    dtPolyRef startRef = 0;
    dtPolyRef endRef = 0;
    dtStatus startStatus = navQuery_->findNearestPoly(start.Data(), extends.Data(), queryFilter, &startRef, nullptr);
    if (dtStatusFailed(startStatus))
    {
#ifdef DEBUG_NAVIGATION
        LOG_WARNING << "findNearestPoly() Failed with startStatus " << startStatus <<
            GetStatusString(startStatus) << std::endl;
#endif
        return false;
    }

    dtStatus endStatus = navQuery_->findNearestPoly(end.Data(), extends.Data(), queryFilter, &endRef, nullptr);
    if (dtStatusFailed(endStatus))
    {
#ifdef DEBUG_NAVIGATION
        LOG_WARNING << "findNearestPoly() Failed with endStatus " << endStatus <<
            GetStatusString(endStatus) << std::endl;
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

    for (int i = 0; i < numPathPoints; ++i)
    {
        dest.push_back(pathData_->pathPoints_[i]);
    }

    return true;
}

Math::Vector3 NavigationMesh::FindNearestPoint(const Math::Vector3& point,
    const Math::Vector3& extents,
    const dtQueryFilter* filter, dtPolyRef* nearestRef)
{
    Math::Vector3 nearestPoint;

    dtPolyRef pointRef;
    if (!nearestRef)
        nearestRef = &pointRef;
    navQuery_->findNearestPoly(&point.x_, &extents.x_, filter ? filter : queryFilter_.get(), nearestRef, &nearestPoint.x_);
    return *nearestRef ? nearestPoint : point;
}

bool NavigationMesh::CanStepOn(const Math::Vector3& point, const Math::Vector3& extents, const dtQueryFilter* filter, dtPolyRef* nearestRef)
{
    dtPolyRef pointRef;
    if (!nearestRef)
        nearestRef = &pointRef;
    Math::Vector3 nearestPoint;
    dtStatus status = navQuery_->findNearestPoly(&point.x_, extents.Data(), filter ? filter : queryFilter_.get(), nearestRef, &nearestPoint.x_);
    if (dtStatusFailed(status))
    {
#ifdef DEBUG_NAVIGATION
        LOG_WARNING << "CanStepOn() Failed with status " << status <<
            GetStatusString(status) << std::endl;
#endif
        return false;
    }
    if (*nearestRef == 0)
        // No poly found
        return false;

    return Math::Equals(point.x_, nearestPoint.x_, extents.x_) &&
        Math::Equals(point.y_, nearestPoint.y_, extents.y_);
}

bool NavigationMesh::FindRandomPoint(Math::Vector3& result, const Math::Vector3& point, float radius, const Math::Vector3& extents,
    const dtQueryFilter* filter)
{
    const dtQueryFilter* queryFilter = filter ? filter : queryFilter_.get();
    dtPolyRef startRef = 0;
    dtStatus startStatus = navQuery_->findNearestPoly(point.Data(), extents.Data(), queryFilter, &startRef, nullptr);
    if (dtStatusFailed(startStatus))
    {
#ifdef DEBUG_NAVIGATION
        LOG_WARNING << "findNearestPoly() Failed with startStatus " << startStatus <<
            GetStatusString(startStatus) << std::endl;
#endif
        return false;
    }

    auto frng = []() -> float
    {
        auto* rng = GetSubsystem<Crypto::Random>();
        return rng->GetFloat();
    };

    dtPolyRef randomRef;
    startStatus = navQuery_->findRandomPointAroundCircle(startRef, point.Data(), radius, queryFilter, frng, &randomRef, &result.x_);
    if (dtStatusFailed(startStatus))
    {
#ifdef DEBUG_NAVIGATION
        LOG_WARNING << "findRandomPointAroundCircle() Failed with startStatus " << startStatus <<
            GetStatusString(startStatus) << std::endl;
#endif
        return false;
    }
    return true;
}

std::string NavigationMesh::GetStatusString(dtStatus status)
{
    std::stringstream ss;
    if (dtStatusDetail(status, DT_WRONG_MAGIC))
        ss << " DT_WRONG_MAGIC";
    if (dtStatusDetail(status, DT_WRONG_VERSION))
        ss << " DT_WRONG_VERSION";
    if (dtStatusDetail(status, DT_OUT_OF_MEMORY))
        ss << " DT_OUT_OF_MEMORY";
    if (dtStatusDetail(status, DT_INVALID_PARAM))
        ss << " DT_INVALID_PARAM";
    if (dtStatusDetail(status, DT_BUFFER_TOO_SMALL))
        ss << " DT_BUFFER_TOO_SMALL";
    if (dtStatusDetail(status, DT_OUT_OF_NODES))
        ss << " DT_OUT_OF_NODES";
    if (dtStatusDetail(status, DT_PARTIAL_RESULT))
        ss << " DT_PARTIAL_RESULT";
    if (dtStatusDetail(status, DT_ALREADY_OCCUPIED))
        ss << " DT_ALREADY_OCCUPIED";
    return ss.str();
}

}
