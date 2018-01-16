#include "stdafx.h"
#include "Map.h"
#include "DataProvider.h"
#include "Game.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "StringHash.h"

namespace Game {

#pragma warning(push)
#pragma warning(disable: 4307)
static constexpr size_t FileTypeScene = Utils::StringHash("Scene");
static constexpr size_t FileTypeNavmesh = Utils::StringHash("NavMesh");
static constexpr size_t FileTypeTerrain = Utils::StringHash("Terrain");

static constexpr size_t AttrName = Utils::StringHash("Name");
static constexpr size_t AttrPosition = Utils::StringHash("Position");
static constexpr size_t AttrRotation = Utils::StringHash("Rotation");
static constexpr size_t AttrScale = Utils::StringHash("Scale");
static constexpr size_t AttrStaticModel = Utils::StringHash("StaticModel");
static constexpr size_t AttrCollisionShape = Utils::StringHash("CollisionShape");

static constexpr size_t AttrShapeType = Utils::StringHash("Shape Type");
static constexpr size_t AttrConvexHull = Utils::StringHash("ConvexHull");
static constexpr size_t AttrModel = Utils::StringHash("Model");
#pragma warning(pop)

Map::Map(std::shared_ptr<Game> game) :
    game_(game),
    navMesh_(nullptr)
{
    octree_ = std::make_unique<Math::Octree>();
}

Map::~Map()
{
}

bool Map::LoadScene(const std::string& name)
{
    std::string file = IO::DataProvider::Instance.GetDataFile(data_.directory + "/" + name);
    pugi::xml_document doc;
    const pugi::xml_parse_result& result = doc.load_file(file.c_str());
    if (result.status != pugi::status_ok)
        return false;
    const pugi::xml_node& scene_node = doc.child("scene");
    if (!scene_node)
        return false;

    for (pugi::xml_node_iterator it = scene_node.begin(); it != scene_node.end(); ++it)
    {
        if (strcmp((*it).name(), "node") == 0)
        {
            LoadSceneNode(*it);
        }
    }

    return true;
}

void Map::LoadSceneNode(const pugi::xml_node& node)
{
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
            case AttrPosition:
                pos = Math::Vector3(value_attr.as_string());
                break;
            case AttrRotation:
                rot = Math::Quaternion(value_attr.as_string());
                break;
            case AttrScale:
                scale = Math::Vector3(value_attr.as_string());
                break;
            case AttrName:
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

        for (const auto& comp : node.children("component"))
        {
            const pugi::xml_attribute& type_attr = comp.attribute("type");
            const size_t type_hash = Utils::StringHashRt(type_attr.as_string());
            switch (type_hash)
            {
            case AttrStaticModel:
            {
                std::shared_ptr<GameObject> object = std::make_shared<GameObject>();
                object->transformation_ = Math::Transformation(pos, scale, rot);
                game->AddObjectInternal(object);
                break;
            }
            case AttrCollisionShape:
            {
                for (const auto& attr : comp.children())
                {
                    const pugi::xml_attribute& name_attr = attr.attribute("name");
                    const size_t name_hash = Utils::StringHashRt(name_attr.as_string());
                    const pugi::xml_attribute& value_attr = attr.attribute("value");
                    switch (name_hash)
                    {
                    case AttrShapeType:
                        break;
                    case AttrModel:
                        break;
                    }
                }
            }
            }
        }
    }
}

bool Map::Load()
{
    std::string file = IO::DataProvider::Instance.GetDataFile(data_.directory + "/index.xml");
    pugi::xml_document doc;
    const pugi::xml_parse_result& result = doc.load_file(file.c_str());
    if (result.status != pugi::status_ok)
        return false;
    const pugi::xml_node& index_node = doc.child("index");
    if (!index_node)
        return false;
    bool ret = true;

    for (const auto& file_node : index_node.children("file"))
    {
        const pugi::xml_attribute& type_attr = file_node.attribute("type");
        const size_t type_hash = Utils::StringHashRt(type_attr.as_string());
        const pugi::xml_attribute& src_attr = file_node.attribute("src");
        switch (type_hash)
        {
        case FileTypeScene:
            if (!LoadScene(src_attr.as_string()))
                ret = false;
            break;
        case FileTypeNavmesh:
        {
            navMesh_ = IO::DataProvider::Instance.GetAsset<NavigationMesh>(data_.directory + "/" + std::string(src_attr.as_string()));
            if (!navMesh_)
                ret = false;
            break;
        }
        case FileTypeTerrain:
            //    terrain_ = IO::DataProvider::Instance.GetAsset<Terrain>(data_.directory + "/terrain.obj");
            //    if (!terrain_)
            //        ret = false;
            break;
        }
    }

    return ret;
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
        }
    }

    {
        std::vector<GameObject*> result;
        Math::SphereOctreeQuery query(result, Math::Sphere(minPos.position, 0.5f));
        octree_->GetObjects(query);
        while (result.size() != 0)
        {
            query.sphere_.center_.x_ += 0.5f;
            query.sphere_.center_.z_ += 0.5f;
            octree_->GetObjects(query);
        }
        return{ query.sphere_.center_, minPos.rotation };
    }
}

}
