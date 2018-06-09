#include "stdafx.h"
#include "Map.h"
#include "DataProvider.h"
#include "Game.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "StringHash.h"
#include "Profiler.h"
#include "IOMap.h"

namespace Game {

Map::Map(std::shared_ptr<Game> game) :
    game_(game),
    navMesh_(nullptr)
{
    octree_ = std::make_unique<Math::Octree>();
}

Map::~Map()
{
}

void Map::LoadSceneNode(const pugi::xml_node& node)
{
    // Game load thread
    if (auto game = game_.lock())
    {
        Math::Vector3 pos;
        Math::Vector3 scale;
        Math::Quaternion rot;
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

        std::shared_ptr<GameObject> object;
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
                for (const auto& attr : comp.children())
                {
                    const pugi::xml_attribute& name_attr = attr.attribute("name");
                    const size_t name_hash = Utils::StringHashRt(name_attr.as_string());
                    const pugi::xml_attribute& value_attr = attr.attribute("value");
                    switch (name_hash)
                    {
                    case IO::Map::AttrShapeType:
                        if (object)
                        {
                            object->SetCollisionShape(
                                std::make_unique<Math::CollisionShapeImpl<Math::BoundingBox>>(
                                    Math::ShapeTypeBoundingBox, -0.5f, 0.5f)
                            );
                        }
                        break;
                    case IO::Map::AttrModel:
                        break;
                    }
                }
                if (object && !object->GetCollisionShape())
                {
                    // Unknown shape add default shape
                    object->SetCollisionShape(
                        std::make_unique<Math::CollisionShapeImpl<Math::BoundingBox>>(
                            Math::ShapeTypeBoundingBox, -0.5f, 0.5f)
                    );
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
                terrain_->heightMap_->matrix_ = terrain_->transformation_.GetMatrix();
                for (const auto& attr : comp.children())
                {
                    const pugi::xml_attribute& name_attr = attr.attribute("name");
                    const size_t name_hash = Utils::StringHashRt(name_attr.as_string());
                    const pugi::xml_attribute& value_attr = attr.attribute("value");
                    switch (name_hash)
                    {
                    case IO::Map::AttrVertexSpacing:
                        terrain_->heightMap_->spacing_ = Math::Vector3(value_attr.as_string());
                        break;
                    }
                }
            }
            }
        }
    }
}

void Map::Update(uint32_t delta)
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

}
