#pragma once

#include "BaseLevel.h"
#include "AddAccountKeyDialog.h"

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
    SharedPtr<AddAccountKeyDialog> addAccountKeyDialog_;
    void CreateScene() override;
    void HandleCharClicked(StringHash eventType, VariantMap& eventData);
    void HandleCreateCharClicked(StringHash eventType, VariantMap& eventData);
    void HandleBackClicked(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleAddAccountKeyClicked(StringHash eventType, VariantMap& eventData);
    void HandleAccountKeyAdded(StringHash eventType, VariantMap& eventData);
};

