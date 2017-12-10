#include "stdafx.h"
#include "WorldLevel.h"
#include "PropStream.h"
#include <AB/ProtocolCodes.h>
#include "AbEvents.h"
#include "FwClient.h"
#include "LevelManager.h"

WorldLevel::WorldLevel(Context* context) :
    BaseLevel(context)
{
}

void WorldLevel::SubscribeToEvents()
{
    BaseLevel::SubscribeToEvents();
    SubscribeToEvent(AbEvents::E_OBJECT_SPAWN, URHO3D_HANDLER(WorldLevel, HandleObjectSpawn));
    SubscribeToEvent(AbEvents::E_OBJECT_SPAWN_EXISTING, URHO3D_HANDLER(WorldLevel, HandleObjectSpawn));
    SubscribeToEvent(AbEvents::E_OBJECT_DESPAWN, URHO3D_HANDLER(WorldLevel, HandleObjectDespawn));
}

void WorldLevel::HandleObjectSpawn(StringHash eventType, VariantMap& eventData)
{
    uint32_t objectId = static_cast<uint32_t>(eventData[AbEvents::ED_OBJECT_ID].GetInt());
    if (objects_.Contains(objectId))
        return;
    Vector3 pos = eventData[AbEvents::ED_POS].GetVector3();
    float rot = eventData[AbEvents::ED_ROTATION].GetFloat();
    Quaternion direction(0.0f, rot, 0.0f);
    String d = eventData[AbEvents::ED_OBJECT_DATA].GetString();
    PropReadStream data(d.CString(), d.Length());
    SpawnObject(objectId, pos, direction, data);

//    if (eventType != AbEvents::E_OBJECT_SPAWN_EXISTING)
        chatWindow_->AddLine("Object spawn: " + pos.ToString());
}

void WorldLevel::SpawnObject(uint32_t id, const Vector3& position,
    const Quaternion& direction, PropReadStream& data)
{
    uint8_t objectType;
    if (!data.Read<uint8_t>(objectType))
        return;

    FwClient* client = context_->GetSubsystem<FwClient>();
    uint32_t playerId = client->GetPlayerId();
    GameObject* object = nullptr;
    switch (objectType)
    {
    case AB::GameProtocol::ObjectTypePlayer:
        if (playerId == id)
        {
            CreatePlayer(id, position, direction);
            object = player_;
        }
        else
            object = CreateActor(id, position, direction);
        break;
    case AB::GameProtocol::ObjectTypeNpc:
        object = CreateActor(id, position, direction);
        break;
    }
    if (object)
        objects_[id] = object;
}

void WorldLevel::HandleObjectDespawn(StringHash eventType, VariantMap& eventData)
{
    uint32_t objectId = static_cast<uint32_t>(eventData[AbEvents::ED_OBJECT_ID].GetInt());
    GameObject* object = objects_[objectId];
    if (object)
    {
        Node* nd = scene_->GetChild(objectId);
        scene_->RemoveChild(nd);
        objects_.Erase(objectId);
        chatWindow_->AddLine("Object left");
    }
}

Actor* WorldLevel::CreateActor(uint32_t id, const Vector3& position, const Quaternion& direction)
{
    Actor* result = Actor::CreateActor(id, context_, scene_);
    result->objectNode_->SetPosition(position);
    result->objectNode_->SetRotation(direction);
    return result;
}

void WorldLevel::CreatePlayer(uint32_t id, const Vector3& position, const Quaternion& direction)
{
    player_ = Player::CreatePlayer(id, context_, scene_);
    player_->objectNode_->SetPosition(position);
    player_->objectNode_->SetRotation(direction);

    // !!! ???
    player_->objectNode_->SetScale(Vector3(10.0f, 10.0f, 10.0f));

    cameraNode_ = scene_->GetChild("CameraNode");
    if (!cameraNode_)
    {
        cameraNode_ = scene_->CreateChild("CameraNode");
        cameraNode_->SetPosition(position + Vector3(0.0f, 5.0f, -10.0f));
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
    chatWindow_->SetAlignment(HA_LEFT, VA_BOTTOM);

    // Ping
    pingLabel_ = uiRoot_->CreateChild<Text>();
    pingLabel_->SetSize(50, 20);
    pingLabel_->SetAlignment(HA_RIGHT, VA_BOTTOM);
    pingLabel_->SetStyleAuto();
    // Chat
    ResourceCache* cache = GetSubsystem<ResourceCache>();



}
