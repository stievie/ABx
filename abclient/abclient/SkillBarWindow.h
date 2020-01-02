#pragma once

#include <AB/TemplEncoder.h>
#include <Urho3DAll.h>

class SkillBarWindow : public Window
{
    URHO3D_OBJECT(SkillBarWindow, Window)
private:
    SharedArrayPtr<Button> skillButtons_;
    SharedPtr<Button> skill1_;
    SharedPtr<Button> skill2_;
    SharedPtr<Button> skill3_;
    SharedPtr<Button> skill4_;
    SharedPtr<Button> skill5_;
    SharedPtr<Button> skill6_;
    SharedPtr<Button> skill7_;
    SharedPtr<Button> skill8_;
    AB::SkillIndices skills_;
    void SubscribeEvents();
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleSkill1Clicked(StringHash eventType, VariantMap& eventData);
    void HandleSkill2Clicked(StringHash eventType, VariantMap& eventData);
    void HandleSkill3Clicked(StringHash eventType, VariantMap& eventData);
    void HandleSkill4Clicked(StringHash eventType, VariantMap& eventData);
    void HandleSkill5Clicked(StringHash eventType, VariantMap& eventData);
    void HandleSkill6Clicked(StringHash eventType, VariantMap& eventData);
    void HandleSkill7Clicked(StringHash eventType, VariantMap& eventData);
    void HandleSkill8Clicked(StringHash eventType, VariantMap& eventData);
    Button* GetButtonFromIndex(uint32_t index);
public:
    static void RegisterObject(Context* context);

    SkillBarWindow(Context* context);
    ~SkillBarWindow() override;

    void SetSkills(const AB::SkillIndices& skills);
};

