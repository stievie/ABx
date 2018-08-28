#pragma once

class MissionMapWindow : public Window
{
    URHO3D_OBJECT(MissionMapWindow, Window);
public:
    static void RegisterObject(Context* context);

    MissionMapWindow(Context* context);
    ~MissionMapWindow()
    {
        UnsubscribeFromAllEvents();
    }
    void
        OnDragBegin(const IntVector2& position, const IntVector2& screenPosition, int buttons, int qualifiers, Cursor* cursor) override;
private:
    void SubscribeToEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
};

