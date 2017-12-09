#pragma once

#pragma warning( push )
#pragma warning( disable : 4100 4305)
#include <Urho3D/Urho3DAll.h>
#pragma warning( pop )

using namespace Urho3D;

/// User defined events
namespace AbEvents
{
    /// Load next level
    static const StringHash E_SET_LEVEL = StringHash("Set levels");
    static const StringHash E_LEVEL_READY = StringHash("Level Ready");
    static const StringHash E_OBJECT_SPAWN = StringHash("Object Spawn");
    static const StringHash E_OBJECT_DESPAWN = StringHash("Object Despawn");

    static const StringHash ED_OBJECT_ID = StringHash("Object ID");
    static const StringHash ED_OBJECT_DATA = StringHash("Object Data");
    static const StringHash ED_POS = StringHash("Position");
    static const StringHash ED_ROTATION = StringHash("Rotation");
}
