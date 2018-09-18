#pragma once

class MapWindow : public Window
{
private:
    SharedPtr<Sprite> mapSprite_;
    SharedPtr<Texture2D> mapTexture_;
    float scale_;
    void HandleMapGameClicked(StringHash eventType, VariantMap& eventData);
    void HandleScreenMode(StringHash eventType, VariantMap& eventData);
    void FitMap();
    void SetButtonsPos();
public:
    MapWindow(Context* context);
    ~MapWindow();
};

