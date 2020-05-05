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
#include "TerrainPatch.h"
#include "Terrain.h"

namespace Game {

TerrainPatch::TerrainPatch(std::shared_ptr<Terrain> owner,
    const Math::Point<int>& offset,
    const Math::Point<int>& size) :
    GameObject(),
    owner_(owner),
    offset_(offset),
    size_(size)
{
    occluder_ = true;
    occludee_ = true;
    // Does not collide
    collisionMask_ = 0;

    const float originX = static_cast<float>(owner->numPatches_.x_ * size.x_) * 0.5f;
    const float originY = static_cast<float>(owner->numPatches_.y_ * size.y_) * 0.5f;

    const float halfSizeX = static_cast<float>(size.x_) * 0.5f;
    const float offsetSizeX = static_cast<float>(offset.x_) * static_cast<float>(size.x_);
    const float halfSizeY = static_cast<float>(size.y_) * 0.5f;
    const float offsetSizeY = static_cast<float>(offset.y_) * static_cast<float>(size.y_);

    const float _x = (halfSizeX + offsetSizeX) - originX;
    const float _y = (halfSizeY + offsetSizeY) - originY;

    const int rawX = static_cast<int>(_x + originX);
    const int rawY = static_cast<int>(_y + originY);

    transformation_.position_.x_ = _x;
    transformation_.position_.z_ = _y;
    transformation_.position_.y_ = owner->GetHeightMap()->GetRawHeight(rawX, rawY);

    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    for (int y = 0; y < size.y_; ++y)
    {
        for (int x = 0; x < size.x_; ++x)
        {
            const float currY = owner->GetHeightMap()->GetRawHeight(
                x + (rawX - (size.x_ / 2)),
                y + (rawY - (size.y_ / 2)));
            if (currY < minY)
                minY = currY;
            if (currY > maxY)
                maxY = currY;
        }
    }
    const Math::Vector3 bbMin(
        transformation_.position_.x_ - (static_cast<float>(size.x_) / 2.0f),
        minY,
        transformation_.position_.z_ - (static_cast<float>(size.y_) / 2.0f));
    const Math::Vector3 bbMax(
        transformation_.position_.x_ + (static_cast<float>(size.x_) / 2.0f),
        maxY,
        transformation_.position_.z_ + (static_cast<float>(size.y_) / 2.0f));
    boundingBox_ = Math::BoundingBox(bbMin, bbMax);

#ifdef DEBUG_COLLISION
    char buff[256];
    int len = sprintf(buff, "TerrainPatch: %d,%d; pos %s; BB %s",
        offset.x_, offset.y_, transformation_.position_.ToString().c_str(),
        boundingBox_.ToString().c_str());
    name_ = std::string(buff, len);
#else
    char buff[64];
    int len = sprintf(buff, "TerrainPatch: %d,%d", offset.x_, offset.y_);
    name_ = std::string(buff, len);
#endif
}

float TerrainPatch::CastRay(const Math::Vector3& origin, const Math::Vector3& direction, float maxDist) const
{
    const float dt = 0.1f;
    const float mint = 0.001f;
    float lh = 0.0f;
    float ly = 0.0f;
    float t = mint;
    while (t < maxDist)
    {
        const Math::Vector3 p = origin + direction * t;
        const float h = GetHeight(p);
        if (p.y_ < h)
            // Get the intersection distance
            return (t - dt + dt * (lh - ly) / (p.y_ - ly - h + lh));
        lh = h;
        ly = p.y_;
        t += dt;
    }
    return Math::M_INFINITE;
}

void TerrainPatch::ProcessRayQuery(const Math::RayOctreeQuery& query,
    std::vector<Math::RayQueryResult>& results)
{
    if (auto o = owner_.lock())
    {
        const Math::Matrix4 matrix = o->transformation_.GetMatrix();
        const Math::Ray localRay = query.ray_.Transformed(matrix.Inverse());
        const float max = Math::Clamp(
            Math::IsInfinite(query.maxDistance_) ? static_cast<float>(o->patchSize_) : query.maxDistance_,
            0.0f,
            static_cast<float>(o->patchSize_));

        const float distance = CastRay(localRay.origin_, localRay.direction_, max);
        const Math::Vector3 normal = -query.ray_.direction_;

        if (!Math::IsInfinite(distance) && (distance < query.maxDistance_))
        {
#ifdef DEBUG_COLLISION
            LOG_DEBUG << "Raycast hit " << *this << std::endl;
#endif
            Math::RayQueryResult result;
            result.position_ = query.ray_.origin_ + distance * query.ray_.direction_;
            result.normal_ = normal;
            result.distance_ = distance;
            result.object_ = this;
            results.push_back(std::move(result));
        }
#ifdef DEBUG_COLLISION
        else
        {
            LOG_DEBUG << "Raycast no hit with " << *this << " distance = " << distance <<
                " BB " << GetWorldBoundingBox() << std::endl;
            if (!octant_)
                LOG_WARNING << "Octand = null" << std::endl;
        }
#endif
    }
}

Math::BoundingBox TerrainPatch::GetWorldBoundingBox() const
{
    if (auto t = owner_.lock())
        return boundingBox_.Transformed(t->transformation_.GetMatrix());
    return Math::BoundingBox();
}

float TerrainPatch::GetHeight(const Math::Vector3& position) const
{
    // Local coordinates
    if (auto o = owner_.lock())
    {
        const Math::Vector3 pos(
            position.x_ + (static_cast<float>(offset_.x_) * static_cast<float>(o->patchSize_)),
            0.0f,
            position.z_ + (static_cast<float>(offset_.y_) * static_cast<float>(o->patchSize_)));
        return o->GetHeight(pos);
    }
    return 0.0f;
}

}
