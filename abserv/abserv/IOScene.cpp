/**
 * Copyright 2020 Stefan Ascher
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

#include "IOScene.h"
#include "DataProvider.h"
#include <sa/StringTempl.h>
#include <sa/StringHash.h>
#include <absmath/Sphere.h>
#include <abscommon/Profiler.h>

//#define DEBUG_LOAD

namespace IO {

static constexpr size_t VarGroup = 3316982911;

bool IOScene::LoadSceneNode(Game::Scene& asset, const pugi::xml_node& node, const Math::Matrix4& parentMatrix)
{
    using namespace sa::literals;
    auto* dataProvider = GetSubsystem<IO::DataProvider>();

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
        asset.spawnPoints_.push_back({ transform.position_, transform.oriention_, group });
        return true;
    }

    ea::shared_ptr<Game::Model> model;
    ea::unique_ptr<Game::SceneObject> object;
    Game::SceneObject* objectPtr = nullptr;
    Math::Vector3 size = Math::Vector3::One;
    Math::Vector3 offset = Math::Vector3::Zero;
    Math::Quaternion offsetRot = Math::Quaternion::Identity;
    Math::ShapeType collShapeType = Math::ShapeType::None;
    // If we have a rigid body collide by default with everything. That's also Urho3Ds default.
    uint32_t collsionLayer = 1;
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
            object = ea::make_unique<Game::SceneObject>();
            object->name = name;
            object->collisionMask = colisionMask;
            object->transformation = transform;
            object->size = size;
            object->offset = offset;
            object->offsetRot = offsetRot;
            object->collsionShapeType = collShapeType;
#ifdef DEBUG_LOAD
            LOG_DEBUG << name << ": Shape " << (int)object->collsionShapeType << ", Mask " << object->collisionMask << std::endl;
#endif
            {
                objectPtr = object.get();
                asset.objects_.push_back(std::move(object));
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
                            objectPtr->model = model;
                        }
#ifdef DEBUG_COLLISION
                        if (model)
                            LOG_DEBUG << model->fileName_ << ": " << model->GetBoundingBox() << std::endl;
#endif
                    }
                    break;
                }
                case "Is Occluder"_Hash:
                    objectPtr->occluder = valueAttr.as_bool() ? Game::SceneObject::Occlude::Yes : Game::SceneObject::Occlude::No;
                    break;
                case "Can Be Occluded"_Hash:
                    objectPtr->occludee = valueAttr.as_bool() ? Game::SceneObject::Occlude::Yes : Game::SceneObject::Occlude::No;
                    break;
                }
            }
            break;
        }
        case "CollisionShape"_Hash:
        {
            size_t collShape = "Box"_Hash;
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

            switch (collShape)
            {
            case "ConvexHull"_Hash:
            case "Capsule"_Hash:
                collShapeType = Math::ShapeType::ConvexHull;
                break;
            case "TriangleMesh"_Hash:
                collShapeType = Math::ShapeType::TriangleMesh;
                break;
            case "Box"_Hash:
                collShapeType = Math::ShapeType::BoundingBox;
                break;
            case "Sphere"_Hash:
            case "Cylinder"_Hash:
                collShapeType = Math::ShapeType::Sphere;
                break;
            default:
                collShapeType = Math::ShapeType::BoundingBox;
                break;
            }

            if (objectPtr)
            {
                objectPtr->size = size;
                objectPtr->offset = offset;
                objectPtr->offsetRot = offsetRot;
                objectPtr->collsionShapeType = collShapeType;
#ifdef DEBUG_LOAD
                LOG_DEBUG << name << ": Shape " << (int)objectPtr->collsionShapeType << std::endl;
#endif
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
                case "Collision Layer"_Hash:
                    collsionLayer = valueAttr.as_uint();
                    if (objectPtr)
                        objectPtr->collisionLayer = collsionLayer;
                    break;
                case "Collision Mask"_Hash:
                    colisionMask = valueAttr.as_uint();
                    if (objectPtr)
                        objectPtr->collisionMask = colisionMask;
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
                    asset.terrainSpacing_ = Math::Vector3(valueAttr.as_string());
                    break;
                }
            }
        }
        }
    }

    const Math::Matrix4 matrix = static_cast<Math::Matrix4>(transform.GetMatrix());
    for (const auto& child : node.children("node"))
    {
        if (!LoadSceneNode(asset, child, matrix))
            return false;
    }

    return true;
}

bool IOScene::Import(Game::Scene& asset, const std::string& name)
{
    AB_PROFILE;
    pugi::xml_document doc;
    const pugi::xml_parse_result result = doc.load_file(name.c_str());
    if (result.status != pugi::status_ok)
    {
        LOG_ERROR << "Error loading file " << name << ": " << result.description() << std::endl;
        return false;
    }
    pugi::xml_node sceneNode = doc.child("scene");
    if (!sceneNode)
    {
        LOG_WARNING << "There is no scene node in file " << name << std::endl;
        sceneNode = doc.child("node");
        if (!sceneNode)
        {
            LOG_ERROR << "File " << name << " does not have a scene node" << std::endl;
            return false;
        }
    }

    for (const auto& child : sceneNode.children("node"))
    {
        if (!LoadSceneNode(asset, child, Math::Matrix4::Identity))
        {
            LOG_ERROR << "Error loading scene node" << std::endl;
            // Can't continue
            return false;
        }
    }
    return true;
}

}
