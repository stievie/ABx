#pragma once

#include "Map.h"
#include "StringHash.h"

namespace IO {

namespace Map {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4307)
#endif
static constexpr size_t FileTypeScene = Utils::StringHash("Scene");
static constexpr size_t FileTypeNavmesh = Utils::StringHash("NavMesh");
static constexpr size_t FileTypeTerrain = Utils::StringHash("Terrain");

static constexpr size_t AttrName = Utils::StringHash("Name");
static constexpr size_t AttrPosition = Utils::StringHash("Position");
static constexpr size_t AttrRotation = Utils::StringHash("Rotation");
static constexpr size_t AttrScale = Utils::StringHash("Scale");
static constexpr size_t AttrStaticModel = Utils::StringHash("StaticModel");
static constexpr size_t AttrRigidBody = Utils::StringHash("RigidBody");
static constexpr size_t AttrCollisionShape = Utils::StringHash("CollisionShape");
static constexpr size_t AttrTerrain = Utils::StringHash("Terrain");
static constexpr size_t AttrVariables = Utils::StringHash("Variables");

static constexpr size_t VarGroup = 3316982911;

// Components
static constexpr size_t CompStaticModel = Utils::StringHash("StaticModel");
static constexpr size_t CompRigidBody = Utils::StringHash("RigidBody");
static constexpr size_t CompCollisionShape = Utils::StringHash("CollisionShape");
static constexpr size_t CompTerrain = Utils::StringHash("Terrain");

static constexpr size_t AttrSize = Utils::StringHash("Size");
static constexpr size_t AttrOffsetPos = Utils::StringHash("Offset Position");
static constexpr size_t AttrOffsetRot = Utils::StringHash("Offset Rotation");
static constexpr size_t AttrShapeType = Utils::StringHash("Shape Type");
static constexpr size_t AttrCollisionShapeTypeConvexHull = Utils::StringHash("ConvexHull");
static constexpr size_t AttrCollisionShapeTypeTriangleMesh = Utils::StringHash("TriangleMesh");
static constexpr size_t AttrCollisionShapeTypeCapsule = Utils::StringHash("Capsule");
static constexpr size_t AttrCollisionShapeTypeBox = Utils::StringHash("Box");   // Default
static constexpr size_t AttrCollisionShapeTypeSphere = Utils::StringHash("Sphere");
static constexpr size_t AttrCollisionShapeTypeCylinder = Utils::StringHash("Cylinder");
static constexpr size_t AttrModel = Utils::StringHash("Model");
static constexpr size_t AttrVertexSpacing = Utils::StringHash("Vertex Spacing");
static constexpr size_t AttrIsOccluder = Utils::StringHash("Is Occluder");
static constexpr size_t AttrIsOccludee = Utils::StringHash("Can Be Occluded");
static constexpr size_t AttrCollisionMask = Utils::StringHash("Collision Mask");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
}

bool IOMap_Load(Game::Map& map);

}
