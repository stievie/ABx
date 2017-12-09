#pragma once

#pragma warning( push )
#pragma warning( disable : 4100 4305)
#include <Urho3D/Urho3DAll.h>
#pragma warning( pop )

#include "BaseLevel.h"

/// All World maps, Outposts, Combat, Exploreable...
/// These all have the Game UI.
class WorldLevel : public BaseLevel
{
    URHO3D_OBJECT(WorldLevel, BaseLevel);
public:
    WorldLevel(Context* context);
    void CreatePlayer(const Vector3& position, const Quaternion& direction) override;
protected:
    Text* pingLabel_;
    void CreateUI() override;
    void SubscribeToEvents() override;
private:
    void HandleObjectSpawnn(StringHash eventType, VariantMap& eventData);
    void HandleObjectDespawn(StringHash eventType, VariantMap& eventData);
};

