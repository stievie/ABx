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

    transformation_.position_ = Math::Vector3(((float)size.x_ / 2.0f) * (float)offset.x_,
        0.0f,
        ((float)size.y_ / 2.0f) * (float)offset.y_);
    transformation_.position_.y_ = owner_->heightMap_->GetHeight(transformation_.position_);

    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    for (int y = 0; y < size.y_; ++y)
    {
        for (int x = 0; x < size.x_; ++x)
        {
            const float currY = owner->heightMap_->GetRawHeight(x + offset.x_, y);
            if (currY < minY)
                minY = currY;
            if (currY > maxY)
                maxY = currY;
        }
    }
    boundingBox_ = Math::BoundingBox(
        Math::Vector3(transformation_.position_.x_ - size.x_, minY, transformation_.position_.z_ - size.y_),
        Math::Vector3(transformation_.position_.x_ + size.x_, maxY, transformation_.position_.z_ + size.y_));

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
