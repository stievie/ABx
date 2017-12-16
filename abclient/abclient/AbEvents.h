#pragma once

using namespace Urho3D;

/// User defined events
namespace AbEvents
{
    /// Load next level
    static const StringHash E_SET_LEVEL = StringHash("Set levels");
    static const StringHash E_LEVEL_READY = StringHash("Level Ready");
    static const StringHash E_OBJECT_SPAWN = StringHash("Object Spawn");
    static const StringHash E_OBJECT_SPAWN_EXISTING = StringHash("Object Spawn existing");
    static const StringHash E_OBJECT_DESPAWN = StringHash("Object Despawn");
    static const StringHash E_OBJECT_POS_UPDATE = StringHash("Object Pos Update");

    static const StringHash ED_MAP_NAME = StringHash("Map name");

    static const StringHash ED_OBJECT_ID = StringHash("Object ID");
    static const StringHash ED_OBJECT_DATA = StringHash("Object Data");
    static const StringHash ED_POS = StringHash("Position");
    static const StringHash ED_ROTATION = StringHash("Rotation");
    static const StringHash ED_SCALE = StringHash("Scale");
}
