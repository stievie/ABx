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

#include "GameObject.h"
#include <absmath/Point.h>
#include <absmath/BoundingBox.h>
#include <eastl.hpp>

namespace Game {

class Terrain;

class TerrainPatch final : public GameObject
{
private:
    ea::weak_ptr<Terrain> owner_;
    Math::BoundingBox boundingBox_;
    mutable Math::BoundingBox transformedBoundingBox_;
    float CastRay(const Math::Vector3& origin, const Math::Vector3& direction, float maxDist) const;
public:
    TerrainPatch(ea::shared_ptr<Terrain> owner,
        const Math::Point<int>& offset,
        const Math::Point<int>& size);
    ~TerrainPatch() override = default;

    /// Process octree raycast.
    void ProcessRayQuery(const Math::RayOctreeQuery& query, ea::vector<Math::RayQueryResult>& results) override;
    Math::BoundingBox GetWorldBoundingBox() const override;
    Math::BoundingBox GetBoundingBox() const override
    {
        return boundingBox_;
    }
    AB::GameProtocol::GameObjectType GetType() const override
    {
        return AB::GameProtocol::GameObjectType::TerrainPatch;
    }
    float GetHeight(const Math::Vector3& position) const;

    Math::Point<int> offset_;
    Math::Point<int> size_;
};

template <>
inline bool Is<TerrainPatch>(const GameObject& obj)
{
    return obj.GetType() == AB::GameProtocol::GameObjectType::TerrainPatch;
}

}
