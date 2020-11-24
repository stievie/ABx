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

#include "TerrainPatch.h"
#include "Terrain.h"

//#define DEBUG_COLLISION

namespace Game {

TerrainPatch::TerrainPatch(ea::shared_ptr<Terrain> owner,
    const Math::IntVector2& offset,
    const Math::IntVector2& size) :
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

    // Terrain patches are only for layer0, this is the real terrain.
    // Layer2 contains buildings, static objects, which are added to the game
    // as regular game objects.
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
        transformation_.position_.x_ - (static_cast<float>(size.x_) * 0.5f),
        minY,
        transformation_.position_.z_ - (static_cast<float>(size.y_) * 0.5f));
    const Math::Vector3 bbMax(
        transformation_.position_.x_ + (static_cast<float>(size.x_) * 0.5f),
        maxY,
        transformation_.position_.z_ + (static_cast<float>(size.y_) * 0.5f));
    boundingBox_ = Math::BoundingBox(bbMin, bbMax);
    // We can cache it because it does not change, i.e. a Terrain does not move.
    worldBoundingBox_ = boundingBox_.Transformed(owner->transformation_.GetMatrix());

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

float TerrainPatch::CastRay(const Math::Vector3& origin, const Math::Vector3& direction, float maxDist, Math::Vector3& position) const
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
        {
#ifdef DEBUG_COLLISION
//            LOG_DEBUG << *this << ": h " << h << " @ " << p << std::endl;
#endif
            position = { p.x_, h, p.z_ };
            // Get the intersection distance
            return (t - dt + dt * (lh - ly) / (p.y_ - ly - h + lh));
        }
        lh = h;
        ly = p.y_;
        t += dt;
    }
    return Math::M_INFINITE;
}

void TerrainPatch::ProcessRayQuery(const Math::RayOctreeQuery& query,
    ea::vector<Math::RayQueryResult>& results)
{
    if (auto o = owner_.lock())
    {
        const Math::Matrix4 matrix = o->transformation_.GetMatrix();
        const Math::Ray localRay = query.ray_.Transformed(matrix.Inverse());
        const float max = Math::Clamp(
            Math::IsInfinite(query.maxDistance_) ? static_cast<float>(o->patchSize_) : query.maxDistance_,
            0.0f,
            static_cast<float>(o->patchSize_));

        Math::Vector3 position;
        const float distance = CastRay(localRay.origin_, localRay.direction_, max, position);
        position = matrix * position;

        if (!Math::IsInfinite(distance) && (distance < query.maxDistance_))
        {
            Math::RayQueryResult result;
            result.position_ = position;
            result.normal_ = -query.ray_.direction_;
            result.distance_ = distance;
            result.object_ = this;
            results.push_back(std::move(result));
#ifdef DEBUG_COLLISION
            LOG_DEBUG << "Raycast{" << localRay.origin_ << "->" << localRay.direction_ <<
                "} Hit " << *this << " @ " << result.position_ << " distance " << distance << std::endl;
#endif
        }
#ifdef DEBUG_COLLISION
        else
        {
//            LOG_DEBUG << "Raycast no hit with " << *this << " distance = " << distance <<
//                " BB " << GetWorldBoundingBox() << std::endl;
        }
#endif
    }
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
        // Terrain patches are only for layer0, this is the real terrain.
        // Layer2 contains buildings, static objects, which are added to the game
        // as regular game objects.
        return o->GetHeightMap()->GetHeight(pos);
    }
    return -Math::M_INFINITE;
}

}
