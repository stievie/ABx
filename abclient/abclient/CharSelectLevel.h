#pragma once

#pragma warning( push )
#pragma warning( disable : 4100 4305)
#include <Urho3D/Urho3DAll.h>
#pragma warning( pop )

#include "BaseLevel.h"

/// Character select
class CharSelectLevel : public BaseLevel
{
    URHO3D_OBJECT(CharSelectLevel, BaseLevel);
public:
    CharSelectLevel(Context* context);
};

