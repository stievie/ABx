#include "stdafx.h"
#include "Map.h"
#include "DataProvider.h"
#include "Game.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "StringHash.h"
#include "Profiler.h"
#include "IOMap.h"
#include "Model.h"
#include "StringUtils.h"
#include "TerrainPatch.h"

namespace Game {

Map::Map(std::shared_ptr<Game> game) :
    game_(game),
    zone_(game->data_.name),
    aiLock_("gamemap"),
    navMesh_(nullptr),
    octree_(std::make_unique<Math::Octree>())
{ }

Map::~Map() = default;

void Map::CreatePatches()
{
    patches_.clear();
    terrain_->numPatches_.x_ = (terrain_->GetHeightMap()->GetWidth() - 1) / terrain_->patchSize_;
    terrain_->numPatches_.y_ = (terrain_->GetHeightMap()->GetHeight() - 1) / terrain_->patchSize_;
    patches_.reserve(static_cast<size_t>(terrain_->numPatches_.x_) * static_cast<size_t>(terrain_->numPatches_.y_));
    for (int y = 0; y < terrain_->numPatches_.y_; ++y)
    {
        for (int x = 0; x < terrain_->numPatches_.x_; ++x)
        {
            patches_.push_back(
                std::make_shared<TerrainPatch>(terrain_, Math::Point<int>(x, y),
                    Math::Point<int>(terrain_->patchSize_, terrain_->patchSize_))
            );
        }
    }
}

TerrainPatch* Map::GetPatch(unsigned index) const
{
    return index < patches_.size() ? patches_[index].get() : nullptr;
}

TerrainPatch* Map::GetPatch(int x, int z) const
{
    if (x < 0 || x >= terrain_->numPatches_.x_ || z < 0 || z >= terrain_->numPatches_.y_)
        return nullptr;
    else
        return GetPatch((unsigned)(z * terrain_->numPatches_.x_ + x));
}

void Map::LoadSceneNode(const pugi::xml_node& node)
{
    auto dataProvider = GetSubsystem<IO::DataProvider>();
    // Game load thread
    if (auto game = game_.lock())
    {
        Math::Vector3 pos;
        Math::Vector3 scale = Math::Vector3::One;
        Math::Quaternion rot = Math::Quaternion::Identity;
        std::string name;
        std::string group;
        bool isSpawnPoint = false;
        for (const auto& attr : node.children("attribute"))
        {
            const pugi::xml_attribute& name_attr = attr.attribute("name");
            const pugi::xml_attribute& value_attr = attr.attribute("value");
            const size_t name_hash = Utils::StringHashRt(name_attr.as_string());
            switch (name_hash)
            {
            case IO::Map::AttrPosition:
                pos = Math::Vector3(value_attr.as_string());
                break;
            case IO::Map::AttrRotation:
                rot = Math::Quaternion(value_attr.as_string()).Normal();
                break;
            case IO::Map::AttrScale:
                scale = Math::Vector3(value_attr.as_string());
                break;
            case IO::Map::AttrName:
                isSpawnPoint = strcmp(value_attr.as_string(), "SpawnPoint") == 0;
                name = value_attr.as_string();
                break;
            case IO::Map::AttrVariables:
            {
                for (const auto& var : attr)
                {
                    if (var.attribute("hash").as_ullong() == IO::Map::VarGroup)
                    {
                        group = var.attribute("value").as_string();
                        break;
                    }
                }
                break;
            }
            default:
                continue;
            }
        }

        if (isSpawnPoint)
        {
#ifdef DEBUG_GAME
//            LOG_DEBUG << "Spawn point: " << group << "; Pos: " << pos.ToString() << std::endl;
#endif
            spawnPoints_.push_back({ pos, rot, group });
            return;
        }

        std::shared_ptr<Model> model;
        std::shared_ptr<GameObject> object;
        Math::Vector3 size = Math::Vector3::One;
        Math::Vector3 offset = Math::Vector3::Zero;
        Math::Quaternion offsetRot = Math::Quaternion::Identity;
        for (const auto& comp : node.children("component"))
        {
            const pugi::xml_attribute& type_attr = comp.attribute("type");
            const size_t type_hash = Utils::StringHashRt(type_attr.as_string());
            // StaticModel must be first component
            switch (type_hash)
            {
            case IO::Map::AttrStaticModel:
            {
                object = std::make_shared<GameObject>();
                object->name_ = name;
                object->collisionMask_ = 0;
                object->transformation_ = Math::Transformation(pos, scale, rot);
                {
                    std::lock_guard<std::mutex> lock(lock_);
                    game->AddObjectInternal(object);
                }
                for (const auto& attr : comp.children())
                {
                    const pugi::xml_attribute& name_attr = attr.attribute("name");
                    const size_t name_hash = Utils::StringHashRt(name_attr.as_string());
                    const pugi::xml_attribute& value_attr = attr.attribute("value");
                    switch (name_hash)
                    {
                    case IO::Map::AttrModel:
                    {
                        std::string modelValue = value_attr.as_string();
                        std::vector<std::string> modelFile = Utils::Split(modelValue, ";");
                        if (modelFile.size() == 2)
                        {
                            if (dataProvider->Exists<Model>(modelFile[1]))
                                // Model is optional
                                model = dataProvider->GetAsset<Model>(modelFile[1]);
#ifdef DEBUG_COLLISION
                            if (model)
                                LOG_DEBUG << model->fileName_ << ": " << model->GetBoundingBox().ToString() << std::endl;
#endif
                        }
                        break;
                    }
                    case IO::Map::AttrIsOccluder:
                        object->occluder_ = value_attr.as_bool();
                        break;
                    case IO::Map::AttrIsOccludee:
                        object->occludee_ = value_attr.as_bool();
                        break;
                    }
                }
                break;
            }
            case IO::Map::AttrCollisionShape:
            {
                size_t coll_shape = IO::Map::AttrCollisionShapeTypeBox;
                if (object)
                {
                    for (const auto& attr : comp.children())
                    {
                        const pugi::xml_attribute& name_attr = attr.attribute("name");
                        const size_t name_hash = Utils::StringHashRt(name_attr.as_string());
                        const pugi::xml_attribute& value_attr = attr.attribute("value");
                        const size_t value_hash = Utils::StringHashRt(value_attr.as_string());
                        switch (name_hash)
                        {
                        case IO::Map::AttrSize:
                            size = Math::Vector3(value_attr.as_string());
                            break;
                        case IO::Map::AttrOffsetPos:
                            offset = Math::Vector3(value_attr.as_string());
                            break;
                        case IO::Map::AttrOffsetRot:
                            offsetRot = Math::Quaternion(value_attr.as_string()).Normal();
                            break;
                        case IO::Map::AttrShapeType:
                        {
                            coll_shape = value_hash;
                        }
                        }
                    }

                    if (object && !object->GetCollisionShape())
                    {
                        if (coll_shape == IO::Map::AttrCollisionShapeTypeTriangleMesh ||
                            coll_shape == IO::Map::AttrCollisionShapeTypeConvexHull ||
                            coll_shape == IO::Map::AttrCollisionShapeTypeCapsule)
                        {
                            if (model)
                            {
#ifdef DEBUG_COLLISION
                                LOG_DEBUG << "Setting ConvexHull collision shape for " << object->GetName() << std::endl;
#endif
                                object->SetCollisionShape(
                                    std::make_unique<Math::CollisionShapeImpl<Math::ConvexHull>>(
                                        Math::ShapeTypeConvexHull, model->shape_->vertexData_)
                                );
                            }
                        }
                        else if (coll_shape == IO::Map::AttrCollisionShapeTypeBox && size != Math::Vector3::Zero)
                        {
                            // The object has the scaling.
                            const Math::Vector3 halfSize = (size * 0.5f);
                            Math::BoundingBox bb(-halfSize + offset, halfSize + offset);
                            // Add Node and Offset rotation -> absolute orientation
                            bb.orientation_ = rot * offsetRot;
                            // Object has then no rotation
                            object->transformation_.oriention_ = Math::Quaternion::Identity;
#ifdef DEBUG_COLLISION
                            LOG_DEBUG << "Setting BB collision shape for " << object->GetName() <<
                                " min/max " << bb.ToString() <<
                                ", size +/- " << halfSize.ToString() << ", orientation " << bb.orientation_.ToString() << std::endl;
#endif
                            object->SetCollisionShape(
                                std::make_unique<Math::CollisionShapeImpl<Math::BoundingBox>>(
                                    Math::ShapeTypeBoundingBox, bb)
                            );
                        }
                        else if ((coll_shape == IO::Map::AttrCollisionShapeTypeSphere || coll_shape == IO::Map::AttrCollisionShapeTypeCylinder) &&
                            size != Math::Vector3::Zero)
                        {
                            // The object has the scaling.
                            float radius = size.x_ * 0.5f;
                            Math::Sphere sphere(offset, radius);
#ifdef DEBUG_COLLISION
                            LOG_DEBUG << "Setting Sphere collision shape for " << object->GetName() <<
                                " Center " << offset.ToString() <<
                                ", Radius " << radius << std::endl;
#endif
                            object->SetCollisionShape(
                                std::make_unique<Math::CollisionShapeImpl<Math::Sphere>>(
                                    Math::ShapeTypeSphere, sphere)
                            );
                        }
                        else if (model)
                        {
                            // If none of the above set to model bounding box
                            auto bb = model->GetBoundingBox();
                            bb.orientation_ = rot * offsetRot;
                            // Object has then no rotation
                            object->transformation_.oriention_ = Math::Quaternion::Identity;
#ifdef DEBUG_COLLISION
                            LOG_DEBUG << "Setting BB collision shape for " << object->GetName() <<
                                " to model BB " << model->GetBoundingBox().ToString() << " orientation " << bb.orientation_.ToString() << std::endl;
#endif
                            object->SetCollisionShape(
                                std::make_unique<Math::CollisionShapeImpl<Math::BoundingBox>>(
                                    Math::ShapeTypeBoundingBox, bb)
                            );
                        }
                    }
                }
                break;
            }
            case IO::Map::AttrRigidBody:
            {
                for (const auto& attr : comp.children())
                {
                    const pugi::xml_attribute& name_attr = attr.attribute("name");
                    const size_t name_hash = Utils::StringHashRt(name_attr.as_string());
                    const pugi::xml_attribute& value_attr = attr.attribute("value");
                    switch (name_hash)
                    {
                    case IO::Map::AttrCollisionMask:
                        object->collisionMask_ = value_attr.as_uint();
                        break;
                    }
                }
                break;
            }
            case IO::Map::AttrTerrain:
            {
                assert(terrain_);
                terrain_->transformation_ = Math::Transformation(pos, scale, rot);
                terrain_->GetHeightMap()->matrix_ = terrain_->transformation_.GetMatrix();
                for (const auto& attr : comp.children())
                {
                    const pugi::xml_attribute& name_attr = attr.attribute("name");
                    const size_t name_hash = Utils::StringHashRt(name_attr.as_string());
                    const pugi::xml_attribute& value_attr = attr.attribute("value");
                    switch (name_hash)
                    {
                    case IO::Map::AttrVertexSpacing:
                        terrain_->GetHeightMap()->spacing_ = Math::Vector3(value_attr.as_string());
                        break;
                    }
                }
            }
            }
        }
    }
}

void Map::AddGameObject(std::shared_ptr<GameObject> object)
{
    if (auto game = game_.lock())
        game->AddObjectInternal(object);
}

void Map::UpdateAi(uint32_t delta)
{
    zone_.update(delta);
}

void Map::UpdateOctree(uint32_t)
{
    octree_->Update();
}

SpawnPoint Map::GetFreeSpawnPoint()
{
    return GetFreeSpawnPoint(spawnPoints_);
}

SpawnPoint Map::GetFreeSpawnPoint(const std::string& group)
{
    auto sps = GetSpawnPoints(group);
    if (sps.size() == 0)
        // Nothing found, return any spawn point
        return GetFreeSpawnPoint();

    return GetFreeSpawnPoint(sps);
}

SpawnPoint Map::GetFreeSpawnPoint(const std::vector<SpawnPoint>& points)
{
    if (points.size() == 0)
        return EmtpySpawnPoint;

    static auto cleanObjects = [](std::vector<GameObject*>& objects)
    {
        if (objects.size() == 0)
            return;
        // Remove all objects that are not interesting
        objects.erase(std::remove_if(objects.begin(), objects.end(), [](GameObject* current)
        {
            return (current->GetCollisionMask() == 0) ||
                (current->GetType() == AB::GameProtocol::ObjectTypeTerrainPatch);
        }), objects.end());
    };

    size_t minObjects = INFINITE;
    SpawnPoint minPos;
    for (const auto& p : points)
    {
        std::vector<GameObject*> result;
        Math::SphereOctreeQuery query(result, Math::Sphere(p.position, 5.0f));
        octree_->GetObjects(query);
        cleanObjects(result);
        if (result.size() < minObjects)
        {
            minPos = p;
            minObjects = result.size();
            if (minObjects == 0)
                break;
        }
    }

    {
        std::vector<GameObject*> result;
        Math::SphereOctreeQuery query(result, Math::Sphere(minPos.position, 1.0f));
        octree_->GetObjects(query);
        cleanObjects(result);
        while (result.size() != 0)
        {
#ifdef DEBUG_GAME
//            LOG_DEBUG << "In place " << result.size() << " Object: " << result.front()->GetName() <<
//                " Type: " << static_cast<int>(result.front()->GetType()) << std::endl;
#endif
            query.sphere_.center_.x_ += 0.2f;
            query.sphere_.center_.z_ += 0.2f;
            query.sphere_.center_.y_ = terrain_->GetHeight(query.sphere_.center_);
            octree_->GetObjects(query);
            cleanObjects(result);
        }
        return{ query.sphere_.center_, minPos.rotation, minPos.group };
    }
}

const SpawnPoint& Map::GetSpawnPoint(const std::string& group) const
{
    if (spawnPoints_.size() == 0)
        return EmtpySpawnPoint;
    for (const auto& sp : spawnPoints_)
    {
        if (sp.group.compare(group) == 0)
            return sp;
    }
    return EmtpySpawnPoint;
}

std::vector<SpawnPoint> Map::GetSpawnPoints(const std::string& group)
{
    std::vector<SpawnPoint> result;
    for (const auto& sp : spawnPoints_)
    {
        if (sp.group.compare(group) == 0)
            result.push_back(sp);
    }
    return result;
}

bool Map::FindPath(std::vector<Math::Vector3>& dest,
    const Math::Vector3& start, const Math::Vector3& end,
    const Math::Vector3& extends /* = Math::Vector3::One */,
    const dtQueryFilter* filter /* = nullptr */)
{
    if (!navMesh_)
        return false;

    bool res = navMesh_->FindPath(dest, start, end, extends, filter);
    if (res)
    {
        for (auto& v : dest)
            v.y_ = GetTerrainHeight(v);
    }
    return res;
}

Math::Vector3 Map::FindNearestPoint(const Math::Vector3& point,
    const Math::Vector3& extents,
    const dtQueryFilter* filter, dtPolyRef* nearestRef)
{
    if (!navMesh_)
        return point;

    Math::Vector3 res = navMesh_->FindNearestPoint(point, extents, filter, nearestRef);
    res.y_ = GetTerrainHeight(res);
    return res;
}

}
