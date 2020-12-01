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
#include <absmath/TriangleMesh.h>
#include <eastl.hpp>
#include <sa/StringTempl.h>
#include <sa/StringHash.h>
#include "Scene.h"

//#define DEBUG_LOAD

namespace IO {
namespace IOMap {

static void CreateObjects(Game::Map& map)
{
    auto game = map.GetGame();
    if (!game)
        return;

    std::mutex loadNodeLock;
    for (const auto& so : map.scene_->objects_)
    {
        ea::shared_ptr<Game::GameObject> object = ea::make_shared<Game::GameObject>();
        object->SetName(so->name);
        object->collisionLayer_ = so->collisionLayer;
        object->collisionMask_ = so->collisionMask;
        object->transformation_ = so->transformation;
#ifdef DEBUG_LOAD
        LOG_DEBUG << *object << ": Mask " << object->collisionMask_ <<
            " Shape " << (int)so->collsionShapeType << " Transformation" << object->transformation_ << std::endl;
#endif
        if (so->occludee != Game::SceneObject::Occlude::Unset)
            object->SetOccludee(so->occludee == Game::SceneObject::Occlude::Yes);
        if (so->occluder != Game::SceneObject::Occlude::Unset)
            object->SetOccluder(so->occludee == Game::SceneObject::Occlude::Yes);
        switch (so->collsionShapeType)
        {
        case Math::ShapeType::BoundingBox:
        {
            // The object has the scaling.
            const Math::Vector3 halfSize = (so->size * 0.5f);
            Math::BoundingBox bb(-halfSize + so->offset, halfSize + so->offset);
            // Add Node and Offset rotation -> absolute orientation
            bb.orientation_ = so->transformation.oriention_ * so->offsetRot;
            // Object has then no rotation
            object->transformation_.oriention_ = Math::Quaternion::Identity;
            object->SetCollisionShape(
                ea::make_unique<Math::CollisionShape<Math::BoundingBox>>(
                    Math::ShapeType::BoundingBox, bb)
            );
#ifdef DEBUG_LOAD
            LOG_DEBUG << *object << ": BB " << bb << std::endl;
#endif
            break;
        }
        case Math::ShapeType::ConvexHull:
            object->SetCollisionShape(
                ea::make_unique<Math::CollisionShape<Math::ConvexHull>>(
                    Math::ShapeType::ConvexHull, so->model->GetShape()->vertexData_)
            );
#ifdef DEBUG_LOAD
            LOG_DEBUG << *object << ": ConvexHull " << std::endl;
#endif
            break;
        case Math::ShapeType::HeightMap:
            break;
        case Math::ShapeType::Sphere:
        {
            float radius = so->size.x_ * 0.5f;
            Math::Sphere sphere(so->offset, radius);
            object->SetCollisionShape(
                ea::make_unique<Math::CollisionShape<Math::Sphere>>(
                    Math::ShapeType::Sphere, sphere)
            );
#ifdef DEBUG_LOAD
            LOG_DEBUG << *object << ": Sphere " << sphere << std::endl;
#endif
            break;
        }
        case Math::ShapeType::TriangleMesh:
            object->SetCollisionShape(
                ea::make_unique<Math::CollisionShape<Math::TriangleMesh>>(
                    Math::ShapeType::TriangleMesh, *so->model->GetShape())
            );
#ifdef DEBUG_LOAD
            LOG_DEBUG << *object << ": TriangleMesh " << std::endl;
#endif
            break;
        default:
            break;
        }

        std::scoped_lock lock(loadNodeLock);
        game->AddObjectInternal(std::move(object));
    }
}

static bool LoadScene(Game::Map& map, const std::string& name)
{
    // Game load thread
    auto* dp = GetSubsystem<IO::DataProvider>();
    const std::string file = DataProvider::GetDataFile(Utils::ConcatPath(map.directory_, name));
    map.scene_ = dp->GetAsset<Game::Scene>(file);
    if (!map.scene_)
        return false;

    map.terrain_->SetSpacing(map.scene_->terrainSpacing_);
    CreateObjects(map);
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
    map.terrain_ = dataProv->GetAsset<Game::Terrain>(Utils::ConcatPath(map.directory_, terrainFile));
    if (!map.terrain_)
    {
        LOG_ERROR << "Error loading terrain " << terrainFile << std::endl;
        return false;
    }

    map.navMesh_ = dataProv->GetAsset<Navigation::NavigationMesh>(Utils::ConcatPath(map.directory_, navMeshFile));
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
    map.CreatePatches();
    // After loading the Scene add terrain patches as game objects
    for (size_t i = 0; i < map.GetPatchesCount(); ++i)
    {
        Game::TerrainPatch* patch = map.GetPatch(i);
        ASSERT(patch);
        map.AddGameObject(patch->GetPtr<Game::TerrainPatch>());
    }

    return true;
}

}
}
