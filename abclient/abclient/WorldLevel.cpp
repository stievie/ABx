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
    chatWindow_->AddLine("Object spawn");
}

void WorldLevel::HandleObjectDespawn(StringHash eventType, VariantMap& eventData)
{
    uint32_t objectId = static_cast<uint32_t>(eventData[AbEvents::ED_OBJECT_ID].GetInt());
    GameObject* object = objects_[objectId];
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
    uiRoot_->RemoveAllChildren();
    BaseLevel::CreateUI();
    chatWindow_ = uiRoot_->CreateChild<ChatWindow>();
    chatWindow_->SetAlignment(HA_LEFT, VA_CUSTOM);

    // Ping
    pingLabel_ = uiRoot_->CreateChild<Text>();
    pingLabel_->SetSize(50, 20);
    pingLabel_->SetAlignment(HA_RIGHT, VA_BOTTOM);
    pingLabel_->SetStyleAuto();
    // Chat
    ResourceCache* cache = GetSubsystem<ResourceCache>();



}
