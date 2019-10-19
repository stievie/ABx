#pragma once

#include "Map.h"
#include <sa/StringHash.h>

namespace IO {

namespace IOMap {
static constexpr size_t FileTypeScene = sa::StringHash("Scene");
static constexpr size_t FileTypeNavmesh = sa::StringHash("NavMesh");
static constexpr size_t FileTypeTerrain = sa::StringHash("Terrain");

static constexpr size_t AttrName = sa::StringHash("Name");
static constexpr size_t AttrPosition = sa::StringHash("Position");
static constexpr size_t AttrRotation = sa::StringHash("Rotation");
static constexpr size_t AttrScale = sa::StringHash("Scale");
static constexpr size_t AttrStaticModel = sa::StringHash("StaticModel");
static constexpr size_t AttrRigidBody = sa::StringHash("RigidBody");
static constexpr size_t AttrCollisionShape = sa::StringHash("CollisionShape");
static constexpr size_t AttrTerrain = sa::StringHash("Terrain");
static constexpr size_t AttrVariables = sa::StringHash("Variables");

static constexpr size_t VarGroup = 3316982911;

// Components
static constexpr size_t CompStaticModel = sa::StringHash("StaticModel");
static constexpr size_t CompRigidBody = sa::StringHash("RigidBody");
static constexpr size_t CompCollisionShape = sa::StringHash("CollisionShape");
static constexpr size_t CompTerrain = sa::StringHash("Terrain");

static constexpr size_t AttrSize = sa::StringHash("Size");
static constexpr size_t AttrOffsetPos = sa::StringHash("Offset Position");
static constexpr size_t AttrOffsetRot = sa::StringHash("Offset Rotation");
static constexpr size_t AttrShapeType = sa::StringHash("Shape Type");
static constexpr size_t AttrCollisionShapeTypeConvexHull = sa::StringHash("ConvexHull");
static constexpr size_t AttrCollisionShapeTypeTriangleMesh = sa::StringHash("TriangleMesh");
static constexpr size_t AttrCollisionShapeTypeCapsule = sa::StringHash("Capsule");
static constexpr size_t AttrCollisionShapeTypeBox = sa::StringHash("Box");   // Default
static constexpr size_t AttrCollisionShapeTypeSphere = sa::StringHash("Sphere");
static constexpr size_t AttrCollisionShapeTypeCylinder = sa::StringHash("Cylinder");
static constexpr size_t AttrModel = sa::StringHash("Model");
static constexpr size_t AttrVertexSpacing = sa::StringHash("Vertex Spacing");
static constexpr size_t AttrIsOccluder = sa::StringHash("Is Occluder");
static constexpr size_t AttrIsOccludee = sa::StringHash("Can Be Occluded");
static constexpr size_t AttrCollisionMask = sa::StringHash("Collision Mask");

bool Load(Game::Map& map);

}
}
