#pragma once

#include <Urho3DAll.h>

class GameMessagesWindow : public UIElement
{
    URHO3D_OBJECT(GameMessagesWindow, UIElement)
private:
    static constexpr float VISIBLE_TIME = 3.0f;
    float visibleTime_;
    SharedPtr<Text> text_;
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
public:
    static void RegisterObject(Context* context);

    GameMessagesWindow(Context* context);
    ~GameMessagesWindow() override;

    void ShowError(const String& message);
};
