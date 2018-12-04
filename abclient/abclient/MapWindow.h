#pragma once

class MapWindow : public Window
{
private:
    static constexpr int BUTTON_SIZE = 32;
    SharedPtr<Sprite> mapSprite_;
    SharedPtr<Texture2D> mapTexture_;
    float scale_;
    float zoom_;
    void HandleMapGameClicked(StringHash eventType, VariantMap& eventData);
    void HandleClicked(StringHash eventType, VariantMap& eventData);
    void HandleScreenMode(StringHash eventType, VariantMap& eventData);
    void FitMap();
    void SetButtonsPos();
public:
    MapWindow(Context* context);
    ~MapWindow();
};

