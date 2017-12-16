#pragma once

#include "WorldLevel.h"

/// PvP combat area
class PvpCombatLevel : public WorldLevel
{
    URHO3D_OBJECT(PvpCombatLevel, BaseLevel);
public:
    PvpCombatLevel(Context* context);
};

