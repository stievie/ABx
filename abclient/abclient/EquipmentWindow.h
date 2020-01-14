#pragma once

#include <Urho3DAll.h>

class Player;

class EquipmentWindow : public Window
{
    URHO3D_OBJECT(EquipmentWindow, Window)
private:
    bool modelLoaded_{ false };
    SharedPtr<Scene> modelScene_;
    SharedPtr<Node> characterNode_;
    SharedPtr<AnimationController> animController_;
    void SubscribeEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    bool LoadObject(uint32_t itemIndex, Node* node);
public:
    EquipmentWindow(Context* context);
    ~EquipmentWindow() override;

    void UpdateEquipment(Player* player);
};
