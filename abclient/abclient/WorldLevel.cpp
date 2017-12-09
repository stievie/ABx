#include "stdafx.h"
#include "WorldLevel.h"
#include "PropStream.h"
#include <AB/ProtocolCodes.h>
#include "AbEvents.h"
#include "FwClient.h"

WorldLevel::WorldLevel(Context* context) :
    BaseLevel(context)
{
}

void WorldLevel::SubscribeToEvents()
{
    BaseLevel::SubscribeToEvents();
    SubscribeToEvent(AbEvents::E_OBJECT_SPAWN, URHO3D_HANDLER(WorldLevel, HandleObjectSpawnn));
    SubscribeToEvent(AbEvents::E_OBJECT_DESPAWN, URHO3D_HANDLER(WorldLevel, HandleObjectDespawn));
}

void WorldLevel::HandleObjectSpawnn(StringHash eventType, VariantMap& eventData)
{
    FwClient* client = context_->GetSubsystem<FwClient>();
    uint32_t playerId = client->GetPlayerId();
    uint32_t objectId = static_cast<uint32_t>(eventData[AbEvents::ED_OBJECT_ID].GetInt());
    Vector3 pos = eventData[AbEvents::ED_POS].GetVector3();
    float rot = eventData[AbEvents::ED_ROTATION].GetFloat();
    Quaternion direction(0.0f, rot, 0.0f);
    String d = eventData[AbEvents::ED_OBJECT_DATA].GetString();
    PropReadStream data(d.CString(), d.Length());

    uint8_t objectType;
    if (!data.Read<uint8_t>(objectType))
        return;

    switch (objectType)
    {
    case AB::GameProtocol::ObjectTypePlayer:
        if (playerId == objectId)
        {
            CreatePlayer(pos, direction);
        }
        break;
    }
}

void WorldLevel::HandleObjectDespawn(StringHash eventType, VariantMap& eventData)
{

}

void WorldLevel::CreatePlayer(const Vector3& position, const Quaternion& direction)
{
    cameraNode_ = scene_->GetChild("CameraNode");
    if (!cameraNode_)
    {
        cameraNode_ = scene_->CreateChild("CameraNode");
        cameraNode_->SetPosition(position);
        cameraNode_->SetRotation(direction);
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
