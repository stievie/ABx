#pragma once

#include "WorldLevel.h"

/// No combat area
class OutpostLevel : public WorldLevel
{
    URHO3D_OBJECT(OutpostLevel, BaseLevel);
public:
    OutpostLevel(Context* context);
protected:
    void SubscribeToEvents() override;
    void CreateUI() override;
private:
    void CreateScene();
};

