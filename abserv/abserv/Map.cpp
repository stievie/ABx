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
    navMesh_(nullptr)
{
    octree_ = std::make_unique<Math::Octree>();
}

Map::~Map() = default;

void Map::CreatePatches()
{
    patches_.clear();
    terrain_->numPatches_.x_ = (terrain_->GetHeightMap()->GetWidth() - 1) / terrain_->patchSize_;
    terrain_->numPatches_.y_ = (terrain_->GetHeightMap()->GetHeight() - 1) / terrain_->patchSize_;
    patches_.reserve(terrain_->numPatches_.x_ * terrain_->numPatches_.y_);
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
    // Game load thread
    if (auto game = game_.lock())
    {
        Math::Vector3 pos;
        Math::Vector3 scale;
        Math::Quaternion rot;
        std::string name;
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
                rot = Math::Quaternion(value_attr.as_string());
                break;
            case IO::Map::AttrScale:
                scale = Math::Vector3(value_attr.as_string());
                break;
            case IO::Map::AttrName:
                isSpawnPoint = strcmp(value_attr.as_string(), "SpawnPoint") == 0;
                name = value_attr.as_string();
                break;
            default:
                continue;
            }
        }

        if (isSpawnPoint)
        {
            spawnPoints_.push_back({ pos, rot });
            return;
        }

        std::shared_ptr<Model> model;
        std::shared_ptr<GameObject> object;
        Math::Vector3 size;
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
                            model = GetSubsystem<IO::DataProvider>()->GetAsset<Model>(modelFile[1]);
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
                        case IO::Map::AttrShapeType:
                        {
                            switch (value_hash)
                            {
                            case IO::Map::AttrTriangleMesh:
                            case IO::Map::AttrConvexHull:
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
                                break;
                            }
                            break;
                        }
                        }
                    }

                    if (object && !object->GetCollisionShape())
                    {
                        // Default BoundingBox
                        if (model)
                        {
#ifdef DEBUG_COLLISION
                            LOG_DEBUG << "Setting BB collision shape for " << object->GetName() <<
                                " to model BB " << model->GetBoundingBox().ToString() << std::endl;
#endif
                            object->SetCollisionShape(
                                std::make_unique<Math::CollisionShapeImpl<Math::BoundingBox>>(
                                    Math::ShapeTypeBoundingBox, model->GetBoundingBox())
                            );
                        }
                        else if (size != Math::Vector3::Zero)
                        {
#ifdef DEBUG_COLLISION
                            LOG_DEBUG << "Setting BB collision shape for " << object->GetName() <<
                                " to size +/- " << size.ToString() << std::endl;
#endif
                            object->SetCollisionShape(
                                std::make_unique<Math::CollisionShapeImpl<Math::BoundingBox>>(
                                    Math::ShapeTypeBoundingBox, Math::BoundingBox(-size, size))
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

void Map::UpdateOctree(uint32_t delta)
{
    AB_UNUSED(delta);
    octree_->Update();
}

SpawnPoint Map::GetFreeSpawnPoint()
{
    if (spawnPoints_.size() == 0)
        return{ Math::Vector3::Zero, Math::Quaternion::Identity };

    size_t minObjects = INFINITE;
    SpawnPoint minPos;
    for (const auto& p : spawnPoints_)
    {
        std::vector<GameObject*> result;
        Math::SphereOctreeQuery query(result, Math::Sphere(p.position, 5.0f));
        octree_->GetObjects(query);
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
        while (result.size() != 0)
        {
            query.sphere_.center_.x_ += 0.5f;
            query.sphere_.center_.z_ += 0.5f;
            query.sphere_.center_.y_ = terrain_->GetHeight(query.sphere_.center_);
            octree_->GetObjects(query);
        }
        return{ query.sphere_.center_, minPos.rotation };
    }
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
