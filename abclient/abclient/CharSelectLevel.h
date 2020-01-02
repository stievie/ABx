#pragma once

#include "BaseLevel.h"

/// Character select
class CharSelectLevel : public BaseLevel
{
    URHO3D_OBJECT(CharSelectLevel, BaseLevel)
public:
    CharSelectLevel(Context* context);
    void CreateCamera();
protected:
    void SubscribeToEvents() override;
    void CreateUI() override;
private:
    void CreateScene() override;
    void HandleCharClicked(StringHash eventType, VariantMap& eventData);
    void HandleCreateCharClicked(StringHash eventType, VariantMap& eventData);
    void HandleBackClicked(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
};

