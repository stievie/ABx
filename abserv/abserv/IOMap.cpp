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


#include "IOMap.h"
#include "DataProvider.h"
#include <pugixml.hpp>
#include "GameObject.h"
#include "TerrainPatch.h"
#include "Model.h"
#include "Game.h"
#include <eastl.hpp>
#include <sa/StringTempl.h>
#include <sa/StringHash.h>

namespace IO {
namespace IOMap {

static constexpr size_t VarGroup = 3316982911;

static bool LoadSceneNode(Game::Map& map, const pugi::xml_node& node, const Math::Matrix4& parentMatrix)
{
    // Game load thread
    using namespace sa::literals;

    auto game = map.GetGame();
    if (!game)
        return false;

    auto* dataProvider = GetSubsystem<IO::DataProvider>();
    std::mutex loadNodeLock;

    Math::Transformation transform;
    Math::Transformation parentTransform;
    parentMatrix.Decompose(&parentTransform.scale_, &parentTransform.oriention_, &parentTransform.position_);

    std::string name;
    std::string group;
    bool isSpawnPoint = false;
    for (const auto& attr : node.children("attribute"))
    {
        const pugi::xml_attribute nameAttr = attr.attribute("name");
        const pugi::xml_attribute valueAttr = attr.attribute("value");
        const size_t nameHash = sa::StringHashRt(nameAttr.as_string());
        switch (nameHash)
        {
        case "Position"_Hash:
            transform.position_ = Math::Vector3(valueAttr.as_string()) + parentTransform.position_;
            break;
        case "Rotation"_Hash:
            transform.oriention_ = Math::Quaternion(valueAttr.as_string()).Normal() * parentTransform.oriention_;
            break;
        case "Scale"_Hash:
            transform.scale_ = Math::Vector3(valueAttr.as_string()) * parentTransform.scale_;
            break;
        case "Name"_Hash:
            isSpawnPoint = strcmp(valueAttr.as_string(), "SpawnPoint") == 0;
            name = valueAttr.as_string();
            break;
        case "Variables"_Hash:
        {
            for (const auto& var : attr)
            {
                if (var.attribute("hash").as_ullong() == VarGroup)
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
       LOG_DEBUG << "Spawn point: " << group << "; Pos: " << transform.position_ << std::endl;
#endif
        // This node is a spawn point. No need to do anything more. SpanwPoints also have no child nodes.
        map.spawnPoints_.push_back({ transform.position_, transform.oriention_, group });
        return true;
    }

    ea::shared_ptr<Game::Model> model;
    ea::shared_ptr<Game::GameObject> object;
    Math::Vector3 size = Math::Vector3::One;
    Math::Vector3 offset = Math::Vector3::Zero;
    Math::Quaternion offsetRot = Math::Quaternion::Identity;
    // If we have a rigid body collide by default with everything. That's also Urho3Ds default.
    uint32_t colisionMask = 0xFFFFFFFF;
    for (const auto& comp : node.children("component"))
    {
        const pugi::xml_attribute type_attr = comp.attribute("type");
        const size_t type_hash = sa::StringHashRt(type_attr.as_string());
        // StaticModel must be first component
        switch (type_hash)
        {
        case "StaticModel"_Hash:
        {
            object = ea::make_shared<Game::GameObject>();
            object->SetName(name);
            object->collisionMask_ = colisionMask;
            object->transformation_ = transform;
            {
                std::scoped_lock lock(loadNodeLock);
                game->AddObjectInternal(object);
            }
            for (const auto& attr : comp.children("attribute"))
            {
                const pugi::xml_attribute& nameAttr = attr.attribute("name");
                const size_t nameHash = sa::StringHashRt(nameAttr.as_string());
                const pugi::xml_attribute& valueAttr = attr.attribute("value");
                switch (nameHash)
                {
                case "Model"_Hash:
                {
                    std::string modelValue = valueAttr.as_string();
                    std::vector<std::string> modelFile = sa::Split(modelValue, ";");
                    if (modelFile.size() == 2)
                    {
                        if (dataProvider->Exists<Game::Model>(modelFile[1]))
                        {
                            // Model is optional
                            model = dataProvider->GetAsset<Game::Model>(modelFile[1]);
                            object->SetModel(model);
                        }
#ifdef DEBUG_COLLISION
                        if (model)
                            LOG_DEBUG << model->fileName_ << ": " << model->GetBoundingBox() << std::endl;
#endif
                    }
                    break;
                }
                case "Is Occluder"_Hash:
                    object->occluder_ = valueAttr.as_bool();
                    break;
                case "Can Be Occluded"_Hash:
                    object->occludee_ = valueAttr.as_bool();
                    break;
                }
            }
            break;
        }
        case "CollisionShape"_Hash:
        {
            size_t collShape = "Box"_Hash;
            if (object)
            {
                for (const auto& attr : comp.children())
                {
                    const pugi::xml_attribute& nameAttr = attr.attribute("name");
                    const size_t nameHash = sa::StringHashRt(nameAttr.as_string());
                    const pugi::xml_attribute& valueAttr = attr.attribute("value");
                    const size_t valueHash = sa::StringHashRt(valueAttr.as_string());
                    switch (nameHash)
                    {
                    case "Size"_Hash:
                        size = Math::Vector3(valueAttr.as_string());
                        break;
                    case "Offset Position"_Hash:
                        offset = Math::Vector3(valueAttr.as_string());
                        break;
                    case "Offset Rotation"_Hash:
                        offsetRot = Math::Quaternion(valueAttr.as_string()).Normal();
                        break;
                    case "Shape Type"_Hash:
                    {
                        collShape = valueHash;
                    }
                    }
                }

                if (object && !object->GetCollisionShape())
                {
                    if (collShape == "ConvexHull"_Hash ||
                        collShape == "Capsule"_Hash)
                    {
                        if (model)
                        {
#ifdef DEBUG_COLLISION
                            LOG_DEBUG << "Setting ConvexHull collision shape for " << *object << std::endl;
#endif
                            object->SetCollisionShape(
                                ea::make_unique<Math::CollisionShape<Math::ConvexHull>>(
                                    Math::ShapeType::ConvexHull, model->GetShape()->vertexData_)
                            );
                        }
                    }
                    else if (collShape == "TriangleMesh"_Hash)
                    {
                        // TODO: This!
                    }
                    else if (collShape == "Box"_Hash && size != Math::Vector3::Zero)
                    {
                        // The object has the scaling.
                        const Math::Vector3 halfSize = (size * 0.5f);
                        Math::BoundingBox bb(-halfSize + offset, halfSize + offset);
                        // Add Node and Offset rotation -> absolute orientation
                        bb.orientation_ = transform.oriention_ * offsetRot;
                        // Object has then no rotation
                        object->transformation_.oriention_ = Math::Quaternion::Identity;
#ifdef DEBUG_COLLISION
                        LOG_DEBUG << "Setting BB collision shape for " << *object <<
                            " min/max " << bb <<
                            ", size +/- " << halfSize << ", orientation " << bb.orientation_ << std::endl;
#endif
                        object->SetCollisionShape(
                            ea::make_unique<Math::CollisionShape<Math::BoundingBox>>(
                                Math::ShapeType::BoundingBox, bb)
                        );
                    }
                    else if ((collShape == "Sphere"_Hash || collShape == "Cylinder"_Hash) &&
                        size != Math::Vector3::Zero)
                    {
                        // The object has the scaling.
                        float radius = size.x_ * 0.5f;
                        Math::Sphere sphere(offset, radius);
#ifdef DEBUG_COLLISION
                        LOG_DEBUG << "Setting Sphere collision shape for " << *object <<
                            " Center " << offset <<
                            ", Radius " << radius << std::endl;
#endif
                        object->SetCollisionShape(
                            ea::make_unique<Math::CollisionShape<Math::Sphere>>(
                                Math::ShapeType::Sphere, sphere)
                        );
                    }
                    else if (model)
                    {
                        // If none of the above set to model bounding box
                        auto bb = model->GetBoundingBox();
                        bb.orientation_ = transform.oriention_ * offsetRot;
                        // Object has then no rotation
                        object->transformation_.oriention_ = Math::Quaternion::Identity;
#ifdef DEBUG_COLLISION
                        LOG_DEBUG << "Setting BB collision shape for " << *object <<
                            " to model BB " << model->GetBoundingBox() << " orientation " << bb.orientation_ << std::endl;
#endif
                        object->SetCollisionShape(
                            ea::make_unique<Math::CollisionShape<Math::BoundingBox>>(
                                Math::ShapeType::BoundingBox, bb)
                        );
                    }
                }
            }
            break;
        }
        case "RigidBody"_Hash:
        {
            for (const auto& attr : comp.children())
            {
                const pugi::xml_attribute& nameAttr = attr.attribute("name");
                const size_t nameHash = sa::StringHashRt(nameAttr.as_string());
                const pugi::xml_attribute& valueAttr = attr.attribute("value");
                switch (nameHash)
                {
                case "Collision Mask"_Hash:
                    colisionMask = valueAttr.as_uint();
                    if (object)
                        object->collisionMask_ = colisionMask;
                    break;
                }
            }
            break;
        }
        case "Terrain"_Hash:
        {
            for (const auto& attr : comp.children())
            {
                const pugi::xml_attribute& nameAttr = attr.attribute("name");
                const size_t nameHash = sa::StringHashRt(nameAttr.as_string());
                const pugi::xml_attribute& valueAttr = attr.attribute("value");
                switch (nameHash)
                {
                case "Vertex Spacing"_Hash:
                    map.terrain_->GetHeightMap()->spacing_ = Math::Vector3(valueAttr.as_string());
                    break;
                }
            }
        }
        }
    }

    const Math::Matrix4 matrix = static_cast<Math::Matrix4>(transform.GetMatrix());
    for (const auto& child : node.children("node"))
    {
        if (!LoadSceneNode(map, child, matrix))
            return false;
    }

    return true;
}

static bool LoadScene(Game::Map& map, const std::string& name)
{
    // Game load thread
    const std::string file = DataProvider::GetDataFile(Utils::ConcatPath(map.directory_, name));
    pugi::xml_document doc;
    const pugi::xml_parse_result result = doc.load_file(file.c_str());
    if (result.status != pugi::status_ok)
    {
        LOG_ERROR << "Error loading file " << file << ": " << result.description() << std::endl;
        return false;
    }
    const pugi::xml_node sceneNode = doc.child("scene");
    if (!sceneNode)
    {
        LOG_ERROR << "File " << file << " does not have a scene node" << std::endl;
        return false;
    }

    for (pugi::xml_node_iterator it = sceneNode.begin(); it != sceneNode.end(); ++it)
    {
        if (strcmp((*it).name(), "node") == 0)
        {
            if (!LoadSceneNode(map, *it, Math::Matrix4::Identity))
            {
                LOG_ERROR << "Error loading scene node" << std::endl;
                // Can't continue
                return false;
            }
        }
    }

    // Update spawn points
    for (auto& p : map.spawnPoints_)
    {
        p.position.y_ = map.terrain_->GetHeight(p.position);
    }

#ifdef DEBUG_GAME
//    LOG_DEBUG << "Spawn points: " << map.spawnPoints_.size() << std::endl;
#endif
    return true;
}

bool Load(Game::Map& map)
{
    AB_PROFILE;
    // Game load thread
    std::string file = DataProvider::GetDataFile(Utils::ConcatPath(map.directory_, "index.xml"));
    pugi::xml_document doc;
    const pugi::xml_parse_result result = doc.load_file(file.c_str());
    if (result.status != pugi::status_ok)
    {
        LOG_ERROR << "Error loading file " << file << ": " << result.description() << std::endl;
        return false;
    }
    const pugi::xml_node indexNode = doc.child("index");
    if (!indexNode)
    {
        LOG_ERROR << "File " << file << " does not have an index node" << std::endl;
        return false;
    }

    auto dataProv = GetSubsystem<IO::DataProvider>();
    std::string sceneFile;
    std::string navMeshFile;
    std::string terrainFile;

    using namespace sa::literals;
    for (const auto& fileNode : indexNode.children("file"))
    {
        const pugi::xml_attribute& typeAttr = fileNode.attribute("type");
        const size_t typeHash = sa::StringHashRt(typeAttr.as_string());
        const pugi::xml_attribute& srcAttr = fileNode.attribute("src");
        switch (typeHash)
        {
        case "Scene"_Hash:
            sceneFile = srcAttr.as_string();
            break;
        case "NavMesh"_Hash:
        {
            navMeshFile = srcAttr.as_string();
            break;
        }
        case "Terrain"_Hash:
            terrainFile = srcAttr.as_string();
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
    map.terrain_ = dataProv->GetAsset<Game::Terrain>(Utils::AddSlash(map.directory_) + terrainFile);
    if (!map.terrain_)
    {
        LOG_ERROR << "Error loading terrain " << terrainFile << std::endl;
        return false;
    }

    map.navMesh_ = dataProv->GetAsset<Navigation::NavigationMesh>(Utils::AddSlash(map.directory_) + navMeshFile);
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
    map.terrain_->GetHeightMap()->ProcessData();
    map.CreatePatches();
    // After loading the Scene add terrain patches as game objects
    for (size_t i = 0; i < map.GetPatchesCount(); ++i)
    {
        // We need a copy of that
        Game::TerrainPatch* patch = map.GetPatch(static_cast<unsigned>(i));
        map.AddGameObject(patch->GetPtr<Game::TerrainPatch>());
    }

    return true;
}

}
}
