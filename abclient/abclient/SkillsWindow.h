#pragma once

#include <Urho3DAll.h>

class Player;
class Actor;
class Spinner;

class SkillsWindow : public Window
{
    URHO3D_OBJECT(SkillsWindow, Window)
private:
    void AddProfessions(const Actor& actor);
    void UpdateAttributes(const Actor& actor);
    void UpdateSkills(const Actor& actor);
    Text* CreateDropdownItem(const String& text, unsigned value);
    void SubscribeEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleLoadClicked(StringHash eventType, VariantMap& eventData);
    void HandleSaveClicked(StringHash eventType, VariantMap& eventData);
    void HandleProfessionSelected(StringHash eventType, VariantMap& eventData);
    void HandleSetAttribValue(StringHash eventType, VariantMap& eventData);
    void HandleSetSecProfession(StringHash eventType, VariantMap& eventData);
    void HandleSkillsChanged(StringHash eventType, VariantMap& eventData);
    UIElement* GetAttributeContainer(uint32_t index);
    LineEdit* GetAttributeEdit(uint32_t index);
    Spinner* GetAttributeSpinner(uint32_t index);
    void SetAttributeValue(uint32_t index, int value);
    void SetProfessionIndex(uint32_t index);
public:
    SkillsWindow(Context* context);
    ~SkillsWindow() override;

    void UpdateAll();
};
