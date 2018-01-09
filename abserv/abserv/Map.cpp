#include "stdafx.h"
#include "Map.h"
#include "DataProvider.h"
#include "Game.h"
#include "Vector3.h"
#include "Quaternion.h"

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

bool Map::LoadObjects()
{
    std::string file = IO::DataProvider::Instance.GetDataFile(data_.directory + "/map.xml");
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
            LoadNode(*it);
        }
    }

    return true;
}

void Map::LoadNode(const pugi::xml_node& node)
{
    if (auto game = game_.lock())
    {
        Math::Vector3 pos;
        Math::Vector3 scale;
        Math::Quaternion rot;
        for (const auto& attr : node.children("attribute"))
        {
            const pugi::xml_attribute& name_attr = attr.attribute("name");
            const pugi::xml_attribute& value_attr = attr.attribute("value");
            if (strcmp(name_attr.as_string(), "Position") == 0)
                pos = Math::Vector3(value_attr.as_string());
            else if (strcmp(name_attr.as_string(), "Rotation") == 0)
                rot = Math::Quaternion(value_attr.as_string());
            else if (strcmp(name_attr.as_string(), "Scale") == 0)
                scale = Math::Vector3(value_attr.as_string());
        }

        for (const auto& comp : node.children("component"))
        {
            const pugi::xml_attribute& type_attr = comp.attribute("type");
            if (strcmp(type_attr.as_string(), "StaticModel") == 0)
            {

                std::shared_ptr<GameObject> object = std::make_shared<GameObject>();
                object->transformation_ = Math::Transformation(pos, scale, rot);
                game->AddObjectInternal(object);
                break;
            }
        }
    }
}

bool Map::Load()
{
    navMesh_ = IO::DataProvider::Instance.GetAsset<NavigationMesh>(data_.directory + "/navmesh.bin");
    if (!navMesh_)
        return false;
//    terrain_ = IO::DataProvider::Instance.GetAsset<Terrain>(data_.directory + "/terrain.obj");
//    if (!terrain_)
//        return false;

    if (!LoadObjects())
        return false;

    return true;
}

void Map::Update(uint32_t delta)
{
    AB_UNUSED(delta);
    octree_->Update();
}

}
