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

#pragma once

#include <eastl.hpp>
#include "Model.h"
#include "Asset.h"
#include <absmath/Transformation.h>
#include <absmath/CollisionShape.h>
#include <absmath/BoundingBox.h>

namespace Game {

struct SceneObject
{
    enum class Occlude
    {
        Unset,
        Yes,
        No
    };
    std::string name;
    Math::Transformation transformation;
    Occlude occluder = Occlude::Unset;
    Occlude occludee = Occlude::Unset;
    Math::BoundingBox boundingBox;
    Math::Vector3 size = Math::Vector3::One;
    Math::Vector3 offset = Math::Vector3::Zero;
    Math::Quaternion offsetRot = Math::Quaternion::Identity;
    uint32_t collisionLayer = 1;
    uint32_t collisionMask = 0xFFFFFFFF;
    Math::ShapeType collsionShapeType = Math::ShapeType::None;
    ea::shared_ptr<Model> model;
};

struct SpawnPoint
{
    Math::Vector3 position;
    Math::Quaternion rotation;
    std::string group;
    bool operator==(const SpawnPoint& rhs) const
    {
        return (position == rhs.position && rotation == rhs.rotation);
    }
    inline bool Empty() const;
};

static const SpawnPoint EmtpySpawnPoint{ Math::Vector3::Zero, Math::Quaternion::Identity, "" };
inline bool SpawnPoint::Empty() const
{
    return *this == EmtpySpawnPoint;
}

// Scene.xml file. Just a container for static scene objects to make it cacheable.
class Scene final : public IO::Asset
{
public:
    Math::Vector3 terrainSpacing_;
    ea::vector<ea::unique_ptr<SceneObject>> objects_;
    ea::vector<SpawnPoint> spawnPoints_;
};

}
