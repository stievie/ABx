#include "stdafx.h"
#include "TerrainPatch.h"
#include "Ray.h"
#include "Vector3.h"
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

    float originX = static_cast<float>(owner->numPatches_.x_ * size.x_) / 2.0f;
    float originY = static_cast<float>(owner->numPatches_.y_ * size.y_) / 2.0f;

    float halfSizeX = static_cast<float>(size.x_) / 2.0f;
    float offsetSizeX = static_cast<float>(offset.x_) * static_cast<float>(size.x_);
    float halfSizeY = static_cast<float>(size.y_) / 2.0f;
    float offsetSizeY = static_cast<float>(offset.y_) * static_cast<float>(size.y_);

    float _x = (halfSizeX + offsetSizeX) - originX;
    float _y = (halfSizeY + offsetSizeY) - originY;

    int rawX = static_cast<int>(_x + originX);
    int rawY = static_cast<int>(_y + originY);

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
    for (float t = mint; t < maxDist; t += dt)
    {
        const Math::Vector3 p = origin + direction * t;
        const float h = GetHeight(p);
        if (p.y_ < h)
            // Get the intersection distance
            return (t - dt + dt * (lh - ly) / (p.y_ - ly - h + lh));
        lh = h;
        ly = p.y_;
    }
    return Math::M_INFINITE;
}

void TerrainPatch::ProcessRayQuery(const Math::RayOctreeQuery& query,
    std::vector<Math::RayQueryResult>& results)
{
    if (auto o = owner_.lock())
    {
        const Math::Matrix4 matrix = o->transformation_.GetMatrix();
        Math::Ray localRay = query.ray_.Transformed(matrix.Inverse());
        float max = Math::Clamp(
            Math::IsInfinite(query.maxDistance_) ? static_cast<float>(o->patchSize_) : query.maxDistance_,
            0.0f,
            static_cast<float>(o->patchSize_));

        float distance = CastRay(localRay.origin_, localRay.direction_, max);
        Math::Vector3 normal = -query.ray_.direction_;

        if (!Math::IsInfinite(distance) && (distance < query.maxDistance_))
        {
#ifdef DEBUG_COLLISION
            LOG_DEBUG << "Raycast hit " << name_ << std::endl;
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
            LOG_DEBUG << "Raycast no hit with " << name_ << " distance = " << distance <<
                " BB " << GetWorldBoundingBox().ToString() << std::endl;
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
