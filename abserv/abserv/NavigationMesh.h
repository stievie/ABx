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

#pragma once

#include <eastl.hpp>
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include "Asset.h"
#include <absmath/Vector3.h>
#include <abscommon/NavigationDef.h>

namespace Navigation {

struct FindPathData;

/// Navigation Mesh constructed from Map and Obstacles
class NavigationMesh final : public IO::Asset
{
private:
    dtNavMesh* navMesh_{ nullptr };
    dtNavMeshQuery* navQuery_;
    ea::unique_ptr<dtQueryFilter> queryFilter_;
    ea::unique_ptr<FindPathData> pathData_;
public:
    NavigationMesh();
    ~NavigationMesh() override;

    static std::string GetStatusString(dtStatus status);

    void SetNavMesh(dtNavMesh* value);

    dtNavMesh* GetNavMesh() const { return navMesh_; }
    dtNavMeshQuery* GetNavQuery() const { return navQuery_; }

    /// Find a path between world space points. Return non-empty list of points if successful.
    /// Extents specifies how far off the navigation mesh the points can be.
    bool FindPath(ea::vector<Math::Vector3>& dest, const Math::Vector3& start, const Math::Vector3& end,
        const Math::Vector3& extends = Math::Vector3::One, const dtQueryFilter* filter = nullptr);
    /// Find the nearest point on the navigation mesh to a given point. Extents specifies how far out from the specified point to check along each axis.
    Math::Vector3 FindNearestPoint(const Math::Vector3& point, const Math::Vector3& extents = Math::Vector3::One,
        const dtQueryFilter* filter = nullptr, dtPolyRef* nearestRef = nullptr);
    bool FindRandomPoint(Math::Vector3& result, const Math::Vector3& point, float radius, const Math::Vector3& extents = Math::Vector3::One,
        const dtQueryFilter* filter = nullptr);
    bool CanStepOn(const Math::Vector3& point, const Math::Vector3& extents = Math::Vector3::One,
        const dtQueryFilter* filter = nullptr, dtPolyRef* nearestRef = nullptr);
    bool GetHeight(float& result, const Math::Vector3& point, const Math::Vector3& extents = Math::Vector3::One,
        const dtQueryFilter* filter = nullptr, dtPolyRef* nearestRef = nullptr);
};

}
