#pragma once

#include "BaseLevel.h"

/// Character creation
class CharCreateLevel : public BaseLevel
{
    URHO3D_OBJECT(CharCreateLevel, BaseLevel);
public:
    CharCreateLevel(Context* context);
};

