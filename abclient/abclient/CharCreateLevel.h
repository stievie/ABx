#pragma once

#include "BaseLevel.h"

/// Character creation
class CharCreateLevel : public BaseLevel
{
    URHO3D_OBJECT(CharCreateLevel, BaseLevel);
public:
    CharCreateLevel(Context* context);
    void CreateCamera();
protected:
    virtual void SubscribeToEvents();
    virtual void CreateUI();
private:
    void CreateScene();
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
};

