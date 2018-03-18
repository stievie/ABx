#pragma once

class MapWindow : public Window
{
private:
    SharedPtr<Sprite> mapSprite_;
    void HandleMapGameClicked(StringHash eventType, VariantMap& eventData);
public:
    MapWindow(Context* context);
    ~MapWindow();
};

