#pragma once

#include "BaseLevel.h"

/// Character select
class CharSelectLevel : public BaseLevel
{
    URHO3D_OBJECT(CharSelectLevel, BaseLevel);
public:
    CharSelectLevel(Context* context);
    void CreateCamera();
protected:
    virtual void SubscribeToEvents();
    virtual void CreateUI();
private:
    void CreateScene();
    void HandleCharClicked(StringHash eventType, VariantMap& eventData);
    void HandleCreateCharClicked(StringHash eventType, VariantMap& eventData);
    void HandleBackClicked(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
};

