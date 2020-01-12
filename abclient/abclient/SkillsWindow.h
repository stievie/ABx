#pragma once

#include <Urho3DAll.h>

class Player;

class SkillsWindow : public Window
{
    URHO3D_OBJECT(SkillsWindow, Window)
private:
    void AddProfessions(const Player& player);
    void UpdateAttributes(const Player& player);
    void UpdateSkills(const Player& player);
    Text* CreateDropdownItem(const String& text, unsigned value);
    void SubscribeEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleProfessionSelected(StringHash eventType, VariantMap& eventData);
public:
    SkillsWindow(Context* context);
    ~SkillsWindow() override;

    void UpdateAll();
};
