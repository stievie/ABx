#pragma once

#pragma warning( push )
#pragma warning( disable : 4100 4305)
#include <Urho3D/Urho3DAll.h>
#pragma warning( pop )

#include "WorldLevel.h"

/// PvP combat area
class PvpCombatLevel : public WorldLevel
{
    URHO3D_OBJECT(PvpCombatLevel, BaseLevel);
public:
    PvpCombatLevel(Context* context);
};

