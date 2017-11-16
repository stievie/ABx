#pragma once

#pragma warning( push )
#pragma warning( disable : 4100 4305)
#include <Urho3D/Urho3DAll.h>
#pragma warning( pop )

#include "BaseLevel.h"

/// Character creation
class CharCreateLevel : public BaseLevel
{
    URHO3D_OBJECT(CharCreateLevel, BaseLevel);
public:
    CharCreateLevel(Context* context);
};

