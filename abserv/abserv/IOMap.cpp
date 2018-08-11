#include "stdafx.h"
#include "IOMap.h"
#include "DataProvider.h"
#include <pugixml.hpp>
#include "Logger.h"
#include "CollisionShape.h"
#include "GameObject.h"
#include "TerrainPatch.h"
#include "Profiler.h"

namespace IO {

bool IOMap::LoadScene(Game::Map& map, const std::string& name)
{
    // Game load thread
    std::string file = IO::DataProvider::Instance.GetDataFile(map.data_.directory + "/" + name);
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
            map.LoadSceneNode(*it);
        }
    }

    // Update spawn points
    for (auto& p : map.spawnPoints_)
    {
        p.position.y_ = map.terrain_->GetHeight(p.position);
    }

    return true;
}

bool IOMap::Load(Game::Map& map)
{
    AB_PROFILE;
    // Game load thread
    std::string file = IO::DataProvider::Instance.GetDataFile(map.data_.directory + "/index.xml");
    pugi::xml_document doc;
    const pugi::xml_parse_result& result = doc.load_file(file.c_str());
    if (result.status != pugi::status_ok)
        return false;
    const pugi::xml_node& index_node = doc.child("index");
    if (!index_node)
        return false;

    std::string sceneFile;
    std::string navMeshFile;
    std::string terrainFile;

    for (const auto& file_node : index_node.children("file"))
    {
        const pugi::xml_attribute& type_attr = file_node.attribute("type");
        const size_t type_hash = Utils::StringHashRt(type_attr.as_string());
        const pugi::xml_attribute& src_attr = file_node.attribute("src");
        switch (type_hash)
        {
        case Map::FileTypeScene:
            sceneFile = src_attr.as_string();
            break;
        case Map::FileTypeNavmesh:
        {
            navMeshFile = src_attr.as_string();
            break;
        }
        case Map::FileTypeTerrain:
            terrainFile = src_attr.as_string();
            break;
        }
    }

    if (sceneFile.empty())
    {
        LOG_ERROR << "Map file " << file << " does not contain a scene" << std::endl;
        return false;
    }
    if (navMeshFile.empty())
    {
        LOG_ERROR << "Map file " << file << " does not contain a nav mesh" << std::endl;
        return false;
    }
    if (terrainFile.empty())
    {
        LOG_ERROR << "Map file " << file << " does not contain a terrain" << std::endl;
        return false;
    }

    // Before scene
    map.terrain_ = IO::DataProvider::Instance.GetAsset<Game::Terrain>(map.data_.directory + "/" + terrainFile);
    if (!map.terrain_)
    {
        LOG_ERROR << "Error loading terrain " << terrainFile << std::endl;
        return false;
    }
    // Add terrain patches as game objects
    for (size_t i = 0; i < map.terrain_->GetPatchesCount(); ++i)
    {
        Game::TerrainPatch* patch = map.terrain_->GetPatch(static_cast<unsigned>(i));
        map.AddGameObject(patch->GetThis<Game::GameObject>());
    }

    map.navMesh_ = IO::DataProvider::Instance.GetAsset<Game::NavigationMesh>(map.data_.directory + "/" + navMeshFile);
    if (!map.navMesh_)
    {
        LOG_ERROR << "Error loading nav mesh " << navMeshFile << std::endl;
        return false;
    }
    if (!LoadScene(map, sceneFile))
    {
        LOG_ERROR << "Error loading scene " << navMeshFile << std::endl;
        return false;
    }

    return true;
}

}
