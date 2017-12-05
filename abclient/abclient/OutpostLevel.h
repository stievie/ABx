#pragma once

#pragma warning( push )
#pragma warning( disable : 4100 4305)
#include <Urho3D/Urho3DAll.h>
#pragma warning( pop )

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
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
};

