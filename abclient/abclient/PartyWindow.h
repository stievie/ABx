#pragma once

enum class PartyWindowMode
{
    ModeOutpost,
    ModeGame
};

class PartyWindow : public Window
{
    URHO3D_OBJECT(PartyWindow, Window);
private:
    PartyWindowMode mode_;
    SharedPtr<Window> window_;
    SharedPtr<LineEdit> addPlayerEdit_;
    void HandleAddTargetClicked(StringHash eventType, VariantMap& eventData);
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleObjectSelected(StringHash eventType, VariantMap& eventData);
    void HandleWindowResized(StringHash eventType, VariantMap& eventData);
    void HandleWindowPositioned(StringHash eventType, VariantMap& eventData);
public:
    static void RegisterObject(Context* context);

    PartyWindow(Context* context);
    ~PartyWindow();

    void SetMode(PartyWindowMode mode);
};

