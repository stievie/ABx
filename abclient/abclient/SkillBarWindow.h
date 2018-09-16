#pragma once

class SkillBarWindow : public Window
{
    URHO3D_OBJECT(SkillBarWindow, Window);
private:
    void SubscribeEvents();
public:
    static void RegisterObject(Context* context);

    SkillBarWindow(Context* context);
    ~SkillBarWindow();
};

