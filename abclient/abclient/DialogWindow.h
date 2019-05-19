#pragma once

class DialogWindow : public Window
{
    URHO3D_OBJECT(DialogWindow, Window);
private:
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
protected:
    virtual void SubscribeEvents();
    void LoadLayout(const String& fileName);
    void Center();
public:
    DialogWindow(Context* context);
    ~DialogWindow();
    virtual void Initialize() { }
};

