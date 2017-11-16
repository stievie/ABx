#pragma once

#pragma warning( push )
#pragma warning( disable : 4100 4305)
#include <Urho3D/Urho3DAll.h>
#pragma warning( pop )

#include "BaseLevel.h"

/// PvP combat area
class PvpCombatLevel : public BaseLevel
{
    URHO3D_OBJECT(PvpCombatLevel, BaseLevel);
public:
    PvpCombatLevel(Context* context);
};

