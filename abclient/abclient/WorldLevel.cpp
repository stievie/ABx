#include "stdafx.h"
#include "WorldLevel.h"
#include "PropStream.h"
#include <AB/ProtocolCodes.h>
#include "AbEvents.h"
#include "FwClient.h"
#include "LevelManager.h"
#include <sstream>

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
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(WorldLevel, HandleMouseDown));
    SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(WorldLevel, HandleMouseUp));
}

void WorldLevel::HandleMouseDown(StringHash eventType, VariantMap& eventData)
{
    using namespace MouseButtonDown;
}

void WorldLevel::HandleMouseUp(StringHash eventType, VariantMap& eventData)
{
    using namespace MouseButtonUp;
}

void WorldLevel::Update(StringHash eventType, VariantMap& eventData)
{
    UNREFERENCED_PARAMETER(eventType);
    UNREFERENCED_PARAMETER(eventData);

    FwClient* c = context_->GetSubsystem<FwClient>();
    std::stringstream s;
    s << "Avg. Ping " << c->GetAvgPing() << " Last Ping " << c->GetLastPing();
    pingLabel_->SetText(String(s.str().c_str()));

    using namespace Update;

    Input* input = GetSubsystem<Input>();

    if (player_)
    {
        // Clear previous controls
        player_->controls_.Set(CTRL_FORWARD | CTRL_BACK | CTRL_LEFT | CTRL_RIGHT | CTRL_JUMP, false);

        // Update controls using keys
        UI* ui = GetSubsystem<UI>();
        if (!ui->GetFocusElement())
        {
            player_->controls_.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
            player_->controls_.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
            player_->controls_.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
            player_->controls_.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
            player_->controls_.Set(CTRL_JUMP, input->GetKeyDown(KEY_SPACE));

            if (input->GetMouseButtonDown(4))
            {
                player_->controls_.yaw_ += (float)input->GetMouseMoveX() * YAW_SENSITIVITY;
                player_->controls_.pitch_ += (float)input->GetMouseMoveY() * YAW_SENSITIVITY;
            }

            // Limit pitch
            player_->controls_.pitch_ = Clamp(player_->controls_.pitch_, -80.0f, 80.0f);
        }
    }
}

void WorldLevel::PostUpdate(StringHash eventType, VariantMap& eventData)
{
    UNREFERENCED_PARAMETER(eventType);
    UNREFERENCED_PARAMETER(eventData);

    if (!player_)
        return;

    Node* characterNode = player_->objectNode_;

    // Get camera lookat dir from character yaw + pitch
    Quaternion rot = Quaternion(player_->controls_.yaw_, Vector3::UP);
    Quaternion dir = rot * Quaternion(player_->controls_.pitch_, Vector3::RIGHT);

    // Third person camera: position behind the character
    Vector3 aimPoint = characterNode->GetPosition() + rot * Vector3(0.0f, 1.0f, -1.0f);
    cameraNode_->SetPosition(aimPoint);
    cameraNode_->SetRotation(dir);
}

void WorldLevel::HandleObjectSpawn(StringHash eventType, VariantMap& eventData)
{
    uint32_t objectId = static_cast<uint32_t>(eventData[AbEvents::ED_OBJECT_ID].GetInt());
    if (objects_.Contains(objectId))
        return;
    Vector3 pos = eventData[AbEvents::ED_POS].GetVector3();
    float rot = eventData[AbEvents::ED_ROTATION].GetFloat();
    Quaternion direction(0.0f, rot, 0.0f);
    Vector3 scale = eventData[AbEvents::ED_SCALE].GetVector3();
    String d = eventData[AbEvents::ED_OBJECT_DATA].GetString();
    PropReadStream data(d.CString(), d.Length());
    SpawnObject(objectId, eventType == AbEvents::E_OBJECT_SPAWN_EXISTING,
        pos, scale, direction, data);
}

void WorldLevel::SpawnObject(uint32_t id, bool existing, const Vector3& position, const Vector3& scale,
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
            CreatePlayer(id, position, scale, direction);
            object = player_;
            object->objectType_ = ObjectTypeSelf;
        }
        else
        {
            object = CreateActor(id, position, scale, direction);
            object->objectType_ = ObjectTypePlayer;
            if (!existing)
                chatWindow_->AddLine("Player joined: " + position.ToString());
        }
        break;
    case AB::GameProtocol::ObjectTypeNpc:
        object = CreateActor(id, position, scale, direction);
        object->objectType_ = ObjectTypeNpc;
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
        object->objectNode_->Remove();
        if (object->objectType_ == ObjectTypePlayer)
            chatWindow_->AddLine("Player left");
        objects_.Erase(objectId);
    }
}

Actor* WorldLevel::CreateActor(uint32_t id, const Vector3& position, const Vector3& scale, const Quaternion& direction)
{
    Actor* result = Actor::CreateActor(id, context_, scene_);
    result->objectNode_->SetPosition(position);
    result->objectNode_->SetRotation(direction);
    result->objectNode_->SetScale(scale);
    return result;
}

void WorldLevel::CreatePlayer(uint32_t id, const Vector3& position, const Vector3& scale, const Quaternion& direction)
{
    player_ = Player::CreatePlayer(id, context_, scene_);
    player_->objectNode_->SetPosition(position);
    player_->objectNode_->SetRotation(direction);
    player_->objectNode_->SetScale(scale);

    cameraNode_ = player_->cameraNode_;
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
}
