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
    void SetScene(SharedPtr<Scene> scene);
private:
    SharedPtr<Texture2D> renderTexture_;
    SharedPtr<Node> cameraNode_;
    SharedPtr<Sprite> mapSprite_;
    int zoom_;
    void FitTexture();
    void SubscribeToEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleResized(StringHash eventType, VariantMap& eventData);
    void HandleVisibleChanged(StringHash eventType, VariantMap& eventData);
};

