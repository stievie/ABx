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
    void SubscribeToEvents() override;
    void CreateUI() override;
private:
    SharedPtr<LineEdit> nameEdit_;
    SharedPtr<DropDownList> professionDropdown_;
    SharedPtr<DropDownList> sexDropdown_;
    SharedPtr<Button> createButton_;
    SharedPtr<Button> cancelButton_;
    void DoCreateCharacter();
    void DoCancel();
    void CreateScene() override;
    Text* CreateDropdownItem(const String& text, uint32_t value);
    Text* CreateDropdownItem(const String& text, const String& value);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleCreateClicked(StringHash eventType, VariantMap& eventData);
    void HandleCancelClicked(StringHash eventType, VariantMap& eventData);
};

