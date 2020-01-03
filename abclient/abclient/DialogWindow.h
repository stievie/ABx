#pragma once

#include <Urho3DAll.h>

class DialogWindow : public Window
{
    URHO3D_OBJECT(DialogWindow, Window)
private:
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
protected:
    UIElement* uiRoot_;
    virtual void SubscribeEvents();
    void LoadLayout(const String& fileName);
    void Center();
public:
    DialogWindow(Context* context);
    ~DialogWindow() override;
    virtual void Initialize() { }
};

