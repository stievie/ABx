#include "stdafx.h"
#include "WorldLevel.h"


WorldLevel::WorldLevel(Context* context) :
    BaseLevel(context)
{
}

void WorldLevel::CreatePlayer(const Vector3& position, const Quaternion& direction)
{
    cameraNode_ = scene_->GetChild("CameraNode");
    if (!cameraNode_)
    {
        cameraNode_ = scene_->CreateChild("CameraNode");
        Camera* camera = cameraNode_->CreateComponent<Camera>();
        camera->SetFarClip(300.0f);
    }
    SetupViewport();
}

void WorldLevel::CreateUI()
{
    BaseLevel::CreateUI();
    // Ping
    pingLabel_ = uiRoot_->CreateChild<Text>();
    pingLabel_->SetSize(50, 20);
    pingLabel_->SetAlignment(HA_RIGHT, VA_BOTTOM);
    pingLabel_->SetStyleAuto();
    // Chat
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Window* window = uiRoot_->CreateChild<Window>();
    uiRoot_->AddChild(window);
    window->SetSize(400, 200);
    window->SetPosition(0, -40);
    window->SetLayoutMode(LM_FREE);
    window->SetLayoutSpacing(10);
    window->SetLayoutBorder(IntRect(10, 10, 10, 10));
    window->SetAlignment(HA_LEFT, VA_BOTTOM);
    window->SetName("Chat");
    window->SetOpacity(0.4f);
    window->SetStyleAuto();

    ListView* chatLog = uiRoot_->CreateChild<ListView>();
    chatLog->SetAlignment(HA_LEFT, VA_TOP);
    chatLog->SetSize(400, 180);
    chatLog->SetStyleAuto();
    chatLog->SetOpacity(0.6f);
    window->AddChild(chatLog);

    LineEdit* nameEdit_ = uiRoot_->CreateChild<LineEdit>();
    nameEdit_->SetName("ChatEdit");
    nameEdit_->SetSize(400, 20);
    nameEdit_->SetMinHeight(20);
    nameEdit_->SetStyleAuto();
    nameEdit_->SetOpacity(0.6f);
    nameEdit_->SetCursorBlinkRate(1.2f);
    nameEdit_->SetAlignment(HA_LEFT, VA_BOTTOM);
    window->AddChild(nameEdit_);

}
