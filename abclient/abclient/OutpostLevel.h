#pragma once

#pragma warning( push )
#pragma warning( disable : 4100 4305)
#include <Urho3D/Urho3DAll.h>
#pragma warning( pop )

#include "BaseLevel.h"

/// No combat area
class OutpostLevel : public BaseLevel
{
    URHO3D_OBJECT(OutpostLevel, BaseLevel);
public:
    OutpostLevel(Context* context);
    void CreatePlayer(const Vector3& position, const Quaternion& direction) override;
protected:
    virtual void SubscribeToEvents();
    virtual void CreateUI();
private:
    void CreateScene();
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
};

