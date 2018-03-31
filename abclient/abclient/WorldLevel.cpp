#include "stdafx.h"
#include "WorldLevel.h"
#include "PropStream.h"
#include <AB/ProtocolCodes.h>
#include "AbEvents.h"
#include "FwClient.h"
#include "LevelManager.h"
#include "MathUtils.h"
#include "TimeUtils.h"

#include <Urho3D/DebugNew.h>

WorldLevel::WorldLevel(Context* context) :
    BaseLevel(context),
    rmbDown_(false),
    mailWindow_(nullptr),
    mapWindow_(nullptr)
{
}

void WorldLevel::SubscribeToEvents()
{
    BaseLevel::SubscribeToEvents();
    SubscribeToEvent(AbEvents::E_OBJECT_SPAWN, URHO3D_HANDLER(WorldLevel, HandleObjectSpawn));
    SubscribeToEvent(AbEvents::E_OBJECT_SPAWN_EXISTING, URHO3D_HANDLER(WorldLevel, HandleObjectSpawn));
    SubscribeToEvent(AbEvents::E_OBJECT_DESPAWN, URHO3D_HANDLER(WorldLevel, HandleObjectDespawn));
    SubscribeToEvent(AbEvents::E_OBJECT_POS_UPDATE, URHO3D_HANDLER(WorldLevel, HandleObjectPosUpdate));
    SubscribeToEvent(AbEvents::E_OBJECT_ROT_UPDATE, URHO3D_HANDLER(WorldLevel, HandleObjectRotUpdate));
    SubscribeToEvent(AbEvents::E_OBJECT_SATE_UPDATE, URHO3D_HANDLER(WorldLevel, HandleObjectStateUpdate));
    SubscribeToEvent(AbEvents::E_OBJECT_SELECTED, URHO3D_HANDLER(WorldLevel, HandleObjectSelected));
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(WorldLevel, HandleMouseDown));
    SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(WorldLevel, HandleMouseUp));
    SubscribeToEvent(E_MOUSEWHEEL, URHO3D_HANDLER(WorldLevel, HandleMouseWheel));
    SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(WorldLevel, HandleMouseMove));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(WorldLevel, HandleKeyDown));
}

SharedPtr<GameObject> WorldLevel::GetObjectAt(const IntVector2& pos)
{
    Ray camRay = GetActiveViewportScreenRay(pos);
    PODVector<RayQueryResult> result;
    Octree* world = scene_->GetComponent<Octree>();
    RayOctreeQuery query(result, camRay, RAY_TRIANGLE, M_INFINITY, DRAWABLE_GEOMETRY);
    // Can not use RaycastSingle because it would also return drawables that are not game objects
    world->Raycast(query);
    if (!result.Empty())
    {
        for (unsigned i = 0; i < result.Size(); i++)
        {
            Node* nd = result[i].node_;
            if (nd)
            {
                SharedPtr<GameObject> obj = GetObjectFromNode(nd);
                if (!obj)
                    continue;

                return obj;
            }
        }
    }
    return SharedPtr<GameObject>();
}

void WorldLevel::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    UI* ui = GetSubsystem<UI>();
    if (ui->GetFocusElement())
        return;

    using namespace KeyDown;
    int key = eventData[P_SCANCODE].GetInt();
    switch (key)
    {
    case SDL_SCANCODE_M:
        ToggleMap();
        break;
    case SDL_SCANCODE_SPACE:
        player_->GotoSelected();
        break;
    }
}

void WorldLevel::HandleMouseDown(StringHash eventType, VariantMap& eventData)
{
    using namespace MouseButtonDown;
    Input* input = GetSubsystem<Input>();
    if (input->GetMouseButtonDown(4))
    {
        rmbDown_ = true;
        mouseDownPos_ = input->GetMousePosition();
        input->SetMouseMode(MM_RELATIVE);
    }
    else if (input->GetMouseButtonDown(1))
    {
        // Pick object
        SharedPtr<GameObject> object = GetObjectAt(input->GetMousePosition());
        if (object && object->IsSelectable())
        {
            SelectObject(object->id_);
        }
    }
}

void WorldLevel::SelectObject(uint32_t objectId)
{
    FwClient* client = context_->GetSubsystem<FwClient>();
    client->SelectObject(player_->id_, objectId);
}

void WorldLevel::HandleMouseUp(StringHash eventType, VariantMap& eventData)
{
    using namespace MouseButtonUp;
    Input* input = GetSubsystem<Input>();
    if (rmbDown_)
    {
        rmbDown_ = false;
        input->SetMousePosition(mouseDownPos_);
        input->SetMouseMode(MM_ABSOLUTE);
    }
}

void WorldLevel::HandleMouseWheel(StringHash eventType, VariantMap& eventData)
{
    using namespace MouseWheel;
    if (player_)
    {
        int delta = eventData[P_WHEEL].GetInt();
        player_->SetCameraDist(delta < 0);
    }
}

void WorldLevel::HandleMouseMove(StringHash eventType, VariantMap& eventData)
{
    using namespace MouseMove;
    // Hover object
    Input* input = GetSubsystem<Input>();
    if (input->GetMouseButtonDown(1) || input->GetMouseButtonDown(4))
        return;

    IntVector2 pos = input->GetMousePosition();
    Ray camRay = GetActiveViewportScreenRay(pos);
    PODVector<RayQueryResult> result;
    Octree* world = scene_->GetComponent<Octree>();
    RayOctreeQuery query(result, camRay);
    // Can not use RaycastSingle because it would return the Zone
    world->Raycast(query);
    if (!result.Empty())
    {
        for (unsigned i = 0; i < result.Size(); i++)
        {
            Node* nd = result[i].node_;
            if (nd)
            {
                SharedPtr<GameObject> obj = GetObjectFromNode(nd);
                if (!obj)
                    continue;
                if (obj == hoveredObject_.Lock())
                    // Still the same object
                    return;
                // Unhover last
                if (auto ho = hoveredObject_.Lock())
                    ho->HoverEnd();
                hoveredObject_ = obj;
                if (auto ho = hoveredObject_.Lock())
                {
                    ho->HoverBegin();
                }
                return;
            }
        }
    }
    if (auto ho = hoveredObject_.Lock())
    {
        ho->HoverEnd();
        hoveredObject_.Reset();
    }
}

void WorldLevel::Update(StringHash eventType, VariantMap& eventData)
{
    UNREFERENCED_PARAMETER(eventType);
    UNREFERENCED_PARAMETER(eventData);

    using namespace Update;

    Input* input = GetSubsystem<Input>();

    if (player_)
    {
        // Clear previous controls
        player_->controls_.Set(CTRL_MOVE_FORWARD | CTRL_MOVE_BACK | CTRL_MOVE_LEFT | CTRL_MOVE_RIGHT |
            CTRL_TURN_LEFT | CTRL_TURN_RIGHT, false);

        // Update controls using keys
        UI* ui = GetSubsystem<UI>();
        if (!ui->GetFocusElement())
        {
            player_->controls_.Set(CTRL_MOVE_FORWARD, input->GetKeyDown(KEY_W) ||
                input->GetKeyDown(KEY_UP));
            player_->controls_.Set(CTRL_MOVE_BACK, input->GetKeyDown(KEY_S) ||
                input->GetKeyDown(KEY_DOWN));
            player_->controls_.Set(CTRL_MOVE_LEFT, input->GetKeyDown(KEY_Q));
            player_->controls_.Set(CTRL_MOVE_RIGHT, input->GetKeyDown(KEY_E));

            if (input->GetMouseButtonDown(4))
            {
                player_->controls_.Set(CTRL_MOVE_LEFT, player_->controls_.IsDown(CTRL_MOVE_LEFT) ||
                    input->GetKeyDown(KEY_A));
                player_->controls_.Set(CTRL_MOVE_RIGHT, player_->controls_.IsDown(CTRL_MOVE_RIGHT) ||
                    input->GetKeyDown(KEY_D));
                player_->controls_.yaw_ += (float)input->GetMouseMoveX() * YAW_SENSITIVITY;
                player_->controls_.pitch_ += (float)input->GetMouseMoveY() * YAW_SENSITIVITY;
            }
            else
            {
                player_->controls_.Set(CTRL_TURN_LEFT, input->GetKeyDown(KEY_A) ||
                    input->GetKeyDown(KEY_LEFT));
                player_->controls_.Set(CTRL_TURN_RIGHT, input->GetKeyDown(KEY_D) ||
                    input->GetKeyDown(KEY_RIGHT));
            }

            // Limit pitch
            player_->controls_.pitch_ = Clamp(player_->controls_.pitch_, -80.0f, 80.0f);
        }
    }
}

void WorldLevel::SetupViewport()
{
    BaseLevel::SetupViewport();
//    postProcess_->SetUseAutoExposure(true);
}

void WorldLevel::CreateScene()
{
    BaseLevel::CreateScene();
    NavigationMesh* navMesh = scene_->GetComponent<NavigationMesh>();
    if (navMesh)
        navMesh->Build();
}

void WorldLevel::HandleObjectSpawn(StringHash eventType, VariantMap& eventData)
{
    uint32_t objectId = static_cast<uint32_t>(eventData[AbEvents::ED_OBJECT_ID].GetInt());
    if (objects_.Contains(objectId))
        return;
    int64_t tick = eventData[AbEvents::ED_UPDATE_TICK].GetInt64();
    Vector3 pos = eventData[AbEvents::ED_POS].GetVector3();
    float rot = eventData[AbEvents::ED_ROTATION].GetFloat();
    Quaternion direction;
    float deg = -rot * (180.0f / (float)M_PI);
    direction.FromAngleAxis(deg, Vector3(0.0f, 1.0f, 0.0f));
    Vector3 scale = eventData[AbEvents::ED_SCALE].GetVector3();
    String d = eventData[AbEvents::ED_OBJECT_DATA].GetString();
    PropReadStream data(d.CString(), d.Length());
    SpawnObject(tick, objectId, eventType == AbEvents::E_OBJECT_SPAWN_EXISTING,
        pos, scale, direction, data);
}

void WorldLevel::SpawnObject(int64_t updateTick, uint32_t id, bool existing, const Vector3& position, const Vector3& scale,
    const Quaternion& rot, PropReadStream& data)
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
            CreatePlayer(id, position, scale, rot);
            object = player_;
            object->objectType_ = ObjectTypeSelf;
        }
        else
        {
            object = CreateActor(id, position, scale, rot);
            object->objectType_ = ObjectTypePlayer;
        }
        break;
    case AB::GameProtocol::ObjectTypeNpc:
        object = CreateActor(id, position, scale, rot);
        object->objectType_ = ObjectTypeNpc;
        break;
    }
    if (object)
    {
        object->Unserialize(data);
        object->spawnTickServer_ = updateTick;
        double now = (Client::AbTick() - updateTick) / 1000.0;
        const float p[3] = { position.x_, position.y_, position.z_ };
        dynamic_cast<Actor*>(object)->posExtrapolator_.Reset(0.0, now, p);
        objects_[id] = object;
        nodeIds_[object->GetNode()->GetID()] = id;
        object->GetNode()->SetName(dynamic_cast<Actor*>(object)->name_);
        switch (object->objectType_)
        {
        case ObjectTypePlayer:
            if (!existing)
                chatWindow_->AddLine(dynamic_cast<Actor*>(object)->name_ + " joined the game", "ChatLogServerInfoText");
            break;
        }
    }
}

void WorldLevel::HandleObjectDespawn(StringHash eventType, VariantMap& eventData)
{
    uint32_t objectId = static_cast<uint32_t>(eventData[AbEvents::ED_OBJECT_ID].GetInt());
    SharedPtr<GameObject> object = objects_[objectId];
    if (object)
    {
        object->RemoveFromScene();
        object->GetNode()->Remove();
        if (object->objectType_ == ObjectTypePlayer)
        {
            Actor* act = dynamic_cast<Actor*>(object.Get());
            chatWindow_->AddLine(act->name_ + " left the game", "ChatLogServerInfoText");
        }
        // If the player has selected this object -> unselect it
        if (player_->GetSelectedObject() == object)
            player_->SelectObject(SharedPtr<GameObject>());
        objects_.Erase(objectId);
    }
}

void WorldLevel::HandleObjectPosUpdate(StringHash eventType, VariantMap& eventData)
{
    uint32_t objectId = static_cast<uint32_t>(eventData[AbEvents::ED_OBJECT_ID].GetInt());
    GameObject* object = objects_[objectId];
    if (object)
    {
        int64_t tick = eventData[AbEvents::ED_UPDATE_TICK].GetInt64();
        double time = (tick - object->spawnTickServer_) / 1000.0;
        Vector3 pos = eventData[AbEvents::ED_POS].GetVector3();
        object->MoveTo(time, pos);
    }
}

void WorldLevel::HandleObjectRotUpdate(StringHash eventType, VariantMap& eventData)
{
    uint32_t objectId = static_cast<uint32_t>(eventData[AbEvents::ED_OBJECT_ID].GetInt());
    GameObject* object = objects_[objectId];
    if (object)
    {
        float rot = eventData[AbEvents::ED_ROTATION].GetFloat();
        bool manual = eventData[AbEvents::ED_ROTATION_MANUAL].GetBool();
        // Manual SetDirection -> don't update camera yaw because it comes from camera move
        object->SetYRotation(rot, !manual);
    }
}

void WorldLevel::HandleObjectStateUpdate(StringHash eventType, VariantMap& eventData)
{
    uint32_t objectId = static_cast<uint32_t>(eventData[AbEvents::ED_OBJECT_ID].GetInt());
    GameObject* object = objects_[objectId];
    if (object)
    {
        int64_t tick = eventData[AbEvents::ED_UPDATE_TICK].GetInt64();
        double time = (tick - object->spawnTickServer_) / 1000.0;
        object->SetCreatureState(time, static_cast<AB::GameProtocol::CreatureState>(eventData[AbEvents::ED_OBJECT_STATE].GetInt()));
    }
}

void WorldLevel::HandleObjectSelected(StringHash eventType, VariantMap& eventData)
{
    uint32_t objectId = static_cast<uint32_t>(eventData[AbEvents::ED_OBJECT_ID].GetInt());
    uint32_t targetId = static_cast<uint32_t>(eventData[AbEvents::ED_OBJECT_ID2].GetInt());
    SharedPtr<GameObject> object = objects_[objectId];
    if (object)
    {
        Actor* actor = dynamic_cast<Actor*>(object.Get());
        if (actor)
        {
            if (targetId != 0)
            {
                SharedPtr<GameObject> target = objects_[targetId];
                if (target)
                {
                    actor->SelectObject(target);
                    if (actor->objectType_ == ObjectTypeSelf)
                    {
                        targetWindow_->SetTarget(target);
                    }
                }
            }
            else
            {
                actor->SelectObject(SharedPtr<GameObject>());
                if (actor->objectType_ == ObjectTypeSelf)
                {
                    targetWindow_->SetTarget(SharedPtr<GameObject>());
                }
            }
        }
    }
}

void WorldLevel::HandleMenuLogout(StringHash eventType, VariantMap& eventData)
{
    gameMenu_->RemoveAllChildren();
    uiRoot_->RemoveChild(gameMenu_);
    FwClient* net = context_->GetSubsystem<FwClient>();
    net->Logout();
    VariantMap& e = GetEventDataMap();
    e[AbEvents::E_SET_LEVEL] = "LoginLevel";
    SendEvent(AbEvents::E_SET_LEVEL, e);
}

void WorldLevel::HandleMenuSelectChar(StringHash eventType, VariantMap& eventData)
{
    gameMenu_->RemoveAllChildren();
    uiRoot_->RemoveChild(gameMenu_);
    FwClient* net = context_->GetSubsystem<FwClient>();
    net->Logout();
    net->Login(net->accountName_, net->accountPass_);
}

void WorldLevel::HandleMenuMail(StringHash eventType, VariantMap& eventData)
{
    MailWindow* mailwnd = new MailWindow(context_);
}

void WorldLevel::HandleTargetWindowUnselectObject(StringHash eventType, VariantMap& eventData)
{
    SelectObject(0);
}

Actor* WorldLevel::CreateActor(uint32_t id, const Vector3& position, const Vector3& scale, const Quaternion& direction)
{
    Actor* result = Actor::CreateActor(id, context_, scene_);
    result->GetNode()->SetPosition(position);
    result->moveToPos_ = position;
    result->GetNode()->SetRotation(direction);
    result->rotateTo_ = direction;
    result->GetNode()->SetScale(scale);
    return result;
}

void WorldLevel::CreatePlayer(uint32_t id, const Vector3& position, const Vector3& scale, const Quaternion& direction)
{
    player_ = Player::CreatePlayer(id, context_, scene_);
    player_->GetNode()->SetPosition(position);
    player_->moveToPos_ = position;
    player_->GetNode()->SetRotation(direction);
    player_->rotateTo_ = direction;
    player_->GetNode()->SetScale(scale);
    player_->UpdateYaw();

    cameraNode_ = player_->cameraNode_;
    // Add sound listener to camera node, also Guild Wars does it so.
    Node* listenerNode = cameraNode_->CreateChild("SoundListenerNode");
    // Let's face the sound
    listenerNode->SetDirection(Vector3(0.0f, M_HALF_PI, 0.0f));
    SoundListener* soundListener = listenerNode->CreateComponent<SoundListener>();
    GetSubsystem<Audio>()->SetListener(soundListener);
    SetupViewport();
}

void WorldLevel::ShowMap()
{
    if (!mapWindow_)
    {
        mapWindow_ = new MapWindow(context_);
    }
    mapWindow_->SetVisible(true);
}

void WorldLevel::HideMap()
{
    if (mapWindow_ && mapWindow_->IsVisible())
        mapWindow_->SetVisible(false);
}

void WorldLevel::ToggleMap()
{
    if (mapWindow_ && mapWindow_->IsVisible())
        mapWindow_->SetVisible(false);
    else
        ShowMap();
}

void WorldLevel::CreateUI()
{
    uiRoot_->RemoveAllChildren();
    BaseLevel::CreateUI();
    chatWindow_ = uiRoot_->CreateChild<ChatWindow>();
    chatWindow_->SetAlignment(HA_LEFT, VA_BOTTOM);

    gameMenu_ = uiRoot_->CreateChild<GameMenu>();
    gameMenu_->SetAlignment(HA_LEFT, VA_TOP);
    SubscribeToEvent(gameMenu_, E_GAMEMENU_LOGOUT, URHO3D_HANDLER(WorldLevel, HandleMenuLogout));
    SubscribeToEvent(gameMenu_, E_GAMEMENU_SELECTCHAR, URHO3D_HANDLER(WorldLevel, HandleMenuSelectChar));
    SubscribeToEvent(gameMenu_, E_GAMEMENU_MAIL , URHO3D_HANDLER(WorldLevel, HandleMenuMail));

    targetWindow_ = uiRoot_->CreateChild<TargetWindow>();
    targetWindow_->SetAlignment(HA_CENTER, VA_TOP);
    targetWindow_->SetVisible(false);
    SubscribeToEvent(targetWindow_, E_TARGETWINDOW_UNSELECT, URHO3D_HANDLER(WorldLevel, HandleTargetWindowUnselectObject));

    // Ping
    pingDot_ = uiRoot_->CreateChild<PingDot>();
    pingDot_->SetSize(IntVector2(24, 24));
    pingDot_->SetAlignment(HA_RIGHT, VA_BOTTOM);
}
