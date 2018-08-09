#include "stdafx.h"
#include "TerrainPatch.h"
#include "Ray.h"
#include "Vector3.h"
#include "Terrain.h"
#include <limits>

namespace Game {

TerrainPatch::TerrainPatch(Terrain* owner, const Math::Point<int>& offset,
    const Math::Point<int>& size) :
    GameObject(),
    owner_(owner),
    offset_(offset),
    size_(size)
{
    occluder_ = true;
    occludee_ = false;
    Math::Vector3 pos((size.x_ / 2.0f) * offset.x_, 0.0f, (size.y_ / 2.0f) * offset.y_);
    pos.y_ = owner_->heightMap_->GetHeight(pos);
    transformation_.position_ = pos;
    float minY = std::numeric_limits<float>::infinity();
    float maxY = -std::numeric_limits<float>::infinity();

    for (int y = 0; y < size.y_; ++y)
    {
        for (int x = 0; x < size.x_; ++x)
        {
            minY = std::min(owner->heightMap_->GetRawHeight(x + offset.x_, y), minY);
            maxY = std::max(owner->heightMap_->GetRawHeight(x + offset.x_, y), maxY);
        }
    }

    boundingBox_ = Math::BoundingBox(
        Math::Vector3(pos.x_ - size.x_, minY, pos.z_ - size.y_),
        Math::Vector3(pos.x_ + size.x_, maxY, pos.z_ + size.y_));
    char buff[64];
    int len = sprintf_s(buff, "TerrainPatch: %d,%d", offset.x_, offset.y_);
    name_ = std::string(buff, len);
}

TerrainPatch::~TerrainPatch()
{
}

void TerrainPatch::ProcessRayQuery(const Math::RayOctreeQuery& query,
    std::vector<Math::RayQueryResult>& results)
{
    Math::Matrix4 matrix = transformation_.GetMatrix();
    Math::Matrix4 inverse(matrix.Inverse());
    Math::Ray localRay = query.ray_.Transformed(inverse);
    float distance = localRay.HitDistance(boundingBox_);
    Math::Vector3 normal = -query.ray_.direction_;

    if (distance < query.maxDistance_)
    {
#ifdef _DEBUG
        LOG_DEBUG << "Raycast hit " << name_ << std::endl;
#endif
        Math::RayQueryResult result;
        result.position_ = query.ray_.origin_ + distance * query.ray_.direction_;
        result.normal_ = normal;
        result.distance_ = distance;
        result.object_ = this;
        results.push_back(result);
    }
}

}
