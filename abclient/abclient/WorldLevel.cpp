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
    mapWindow_(nullptr),
    partyWindow_(nullptr)
{
}

void WorldLevel::SubscribeToEvents()
{
    BaseLevel::SubscribeToEvents();

    SubscribeToEvent(AbEvents::E_SERVERJOINED, URHO3D_HANDLER(WorldLevel, HandleServerJoined));
    SubscribeToEvent(AbEvents::E_SERVERLEFT, URHO3D_HANDLER(WorldLevel, HandleServerLeft));
    SubscribeToEvent(AbEvents::E_OBJECTSPAWN, URHO3D_HANDLER(WorldLevel, HandleObjectSpawn));
    SubscribeToEvent(AbEvents::E_OBJECTDESPAWN, URHO3D_HANDLER(WorldLevel, HandleObjectDespawn));
    SubscribeToEvent(AbEvents::E_OBJECTPOSUPDATE, URHO3D_HANDLER(WorldLevel, HandleObjectPosUpdate));
    SubscribeToEvent(AbEvents::E_OBJECTROTUPDATE, URHO3D_HANDLER(WorldLevel, HandleObjectRotUpdate));
    SubscribeToEvent(AbEvents::E_OBJECTSTATEUPDATE, URHO3D_HANDLER(WorldLevel, HandleObjectStateUpdate));
    SubscribeToEvent(AbEvents::E_OBJECTSELECTED, URHO3D_HANDLER(WorldLevel, HandleObjectSelected));
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(WorldLevel, HandleMouseDown));
    SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(WorldLevel, HandleMouseUp));
    SubscribeToEvent(E_MOUSEWHEEL, URHO3D_HANDLER(WorldLevel, HandleMouseWheel));
    SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(WorldLevel, HandleMouseMove));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(WorldLevel, HandleKeyDown));
    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(WorldLevel, HandleKeyUp));
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
        for (PODVector<RayQueryResult>::ConstIterator it = result.Begin(); it != result.End(); ++it)
        {
            if (Node* nd = (*it).node_)
            {
                if (SharedPtr<GameObject> obj = GetObjectFromNode(nd))
                    return obj;
            }
        }
    }
    return SharedPtr<GameObject>();
}

bool WorldLevel::TerrainRaycast(const IntVector2& pos, Vector3& hitPos)
{
    Ray camRay = GetActiveViewportScreenRay(pos);
    PODVector<RayQueryResult> result;
    Octree* world = scene_->GetComponent<Octree>();
    RayOctreeQuery query(result, camRay, RAY_TRIANGLE, M_INFINITY, DRAWABLE_GEOMETRY);
    // Can not use RaycastSingle because it would also return drawables that are not game objects
    world->RaycastSingle(query);
    if (!result.Empty())
    {
        for (PODVector<RayQueryResult>::ConstIterator it = result.Begin(); it != result.End(); ++it)
        {
            if (dynamic_cast<TerrainPatch*>((*it).drawable_))
            {
                hitPos = (*it).position_;
                return true;
            }
        }
    }
    return false;
}

void WorldLevel::HandleServerJoined(StringHash eventType, VariantMap & eventData)
{
    FwClient* client = GetSubsystem<FwClient>();
    client->UpdateServers();
}

void WorldLevel::HandleServerLeft(StringHash eventType, VariantMap & eventData)
{
    FwClient* client = GetSubsystem<FwClient>();
    client->UpdateServers();
}

void WorldLevel::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    UI* ui = GetSubsystem<UI>();
    if (ui->GetFocusElement())
        return;

    using namespace KeyDown;
    bool repeat = eventData[P_REPEAT].GetBool();
    if (repeat)
        return;

    int scanCode = eventData[P_SCANCODE].GetInt();
    switch (scanCode)
    {
    case SDL_SCANCODE_M:
        ToggleMap();
        break;
    case SDL_SCANCODE_SPACE:
        player_->FollowSelected();
        break;
    }
    int key = eventData[P_KEY].GetInt();
    if (key == KEY_R)
        player_->controls_.Set(CTRL_MOVE_LOCK, !player_->controls_.IsDown(CTRL_MOVE_LOCK));
}

void WorldLevel::HandleKeyUp(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyUp;
}

void WorldLevel::HandleMouseDown(StringHash eventType, VariantMap& eventData)
{
    using namespace MouseButtonDown;

    auto* ui = GetSubsystem<UI>();
    IntVector2 pos = ui->GetCursorPosition();
    // Check the cursor is visible and there is no UI element in front of the cursor
    if (ui->GetElementAt(pos, true))
        return;

    Input* input = GetSubsystem<Input>();
    if (input->GetMouseButtonDown(MOUSEB_RIGHT))
    {
        rmbDown_ = true;
        mouseDownPos_ = input->GetMousePosition();
        input->SetMouseMode(MM_RELATIVE);
    }
    else if (input->GetMouseButtonDown(MOUSEB_LEFT))
    {
        // Pick object
        SharedPtr<GameObject> object = GetObjectAt(input->GetMousePosition());
        if (object)
        {
            player_->ClickObject(object->id_);
            if (object->IsSelectable())
                player_->SelectObject(object->id_);
        }
        else
        {
            // TODO: If not mouse move diabled
            Vector3 p;
            if (TerrainRaycast(input->GetMousePosition(), p))
            {
                player_->GotoPosition(p);
            }
        }
    }
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
    if (input->GetMouseButtonDown(MOUSEB_LEFT) || input->GetMouseButtonDown(MOUSEB_RIGHT))
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
        for (PODVector<RayQueryResult>::ConstIterator it = result.Begin(); it != result.End(); ++it)
        {
            if (Node* nd = (*it).node_)
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
            if (input->GetKeyDown(KEY_W) || input->GetKeyDown(KEY_UP))
            {
                player_->controls_.Set(CTRL_MOVE_FORWARD, true);
                if (!input->GetKeyDown(KEY_R))
                    player_->controls_.Set(CTRL_MOVE_LOCK, false);
            }
            if (input->GetKeyDown(KEY_S) || input->GetKeyDown(KEY_DOWN))
            {
                player_->controls_.Set(CTRL_MOVE_BACK, true);
                if (!input->GetKeyDown(KEY_R))
                    player_->controls_.Set(CTRL_MOVE_LOCK, false);
            }
            player_->controls_.Set(CTRL_MOVE_LEFT, input->GetKeyDown(KEY_Q));
            player_->controls_.Set(CTRL_MOVE_RIGHT, input->GetKeyDown(KEY_E));

            if (input->GetMouseButtonDown(MOUSEB_RIGHT))
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
            if (player_->controls_.yaw_ > 360.0f)
                player_->controls_.yaw_ -= 360.0f;
            else if (player_->controls_.yaw_ < 0.0f)
                player_->controls_.yaw_ += 360.0f;
        }
    }
}

void WorldLevel::SetupViewport()
{
    BaseLevel::SetupViewport();
    postProcess_->SetUseBloomHDR(true);
    postProcess_->SetUseColorCorrection(true);
//    postProcess_->SetAutoExposureLumRange(Vector2(0.5f, 1.0f));
//    postProcess_->SetAutoExposureAdaptRate(0.5f);
//    postProcess_->SetAutoExposureMidGrey(1.0f);
//    postProcess_->SetUseAutoExposure(true);
}

void WorldLevel::CreateScene()
{
    BaseLevel::CreateScene();
    NavigationMesh* navMesh = scene_->GetComponent<NavigationMesh>();
    if (navMesh)
        navMesh->Build();
}

void WorldLevel::PostUpdate(StringHash eventType, VariantMap& eventData)
{
    BaseLevel::PostUpdate(eventType, eventData);
}

void WorldLevel::HandleObjectSpawn(StringHash eventType, VariantMap& eventData)
{
    using namespace AbEvents::ObjectSpawn;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    if (objects_.Contains(objectId))
        return;
    int64_t tick = eventData[P_UPDATETICK].GetInt64();
    const Vector3& pos = eventData[P_POSITION].GetVector3();
    float rot = eventData[P_ROTATION].GetFloat();
    Quaternion direction;
    float deg = -rot * (180.0f / (float)M_PI);
    direction.FromAngleAxis(deg, Vector3(0.0f, 1.0f, 0.0f));
    const Vector3& scale = eventData[P_SCALE].GetVector3();
    AB::GameProtocol::CreatureState state = static_cast<AB::GameProtocol::CreatureState>(eventData[P_STATE].GetUInt());
    const String& d = eventData[P_DATA].GetString();
    bool existing = eventData[P_EXISTING].GetBool();
    PropReadStream data(d.CString(), d.Length());
    SpawnObject(tick, objectId, existing, pos, scale, direction, state, data);
}

void WorldLevel::SpawnObject(int64_t updateTick, uint32_t id, bool existing,
    const Vector3& position, const Vector3& scale,
    const Quaternion& rot, AB::GameProtocol::CreatureState state,
    PropReadStream& data)
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
            CreatePlayer(id, position, scale, rot, state, data);
            object = player_;
            object->objectType_ = ObjectTypeSelf;
        }
        else
        {
            object = CreateActor(id, position, scale, rot, state, data);
            object->objectType_ = ObjectTypePlayer;
        }
        break;
    case AB::GameProtocol::ObjectTypeNpc:
        object = CreateActor(id, position, scale, rot, state, data);
        object->objectType_ = ObjectTypeNpc;
        break;
    }
    if (object)
    {
        object->spawnTickServer_ = updateTick;
        const float p[3] = { position.x_, position.y_, position.z_ };
        // Here an object is always an Actor
        dynamic_cast<Actor*>(object)->posExtrapolator_.Reset(object->GetServerTime(updateTick),
            object->GetClientTime(), p);
        object->GetNode()->SetName(dynamic_cast<Actor*>(object)->name_);
        objects_[id] = object;
        nodeIds_[object->GetNode()->GetID()] = id;
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
    using namespace AbEvents::ObjectDespawn;
    uint32_t objectId = eventData[P_OBJECTID].GetInt();
    GameObject* object = objects_[objectId];
    if (object)
    {
        SharedPtr<GameObject> selO = player_->GetSelectedObject();
        if (selO && selO->id_ == object->id_)
        {
            // If the selected object leaves unselect it
            player_->SetSelectedObject(SharedPtr<GameObject>());
            targetWindow_->SetTarget(SharedPtr<GameObject>());
        }
        object->RemoveFromScene();
        object->GetNode()->Remove();
        if (object->objectType_ == ObjectTypePlayer)
        {
            Actor* act = dynamic_cast<Actor*>(object);
            chatWindow_->AddLine(act->name_ + " left the game", "ChatLogServerInfoText");
        }
        objects_.Erase(objectId);
    }
}

void WorldLevel::HandleObjectPosUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace AbEvents::ObjectPosUpdate;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    GameObject* object = objects_[objectId];
    if (object)
    {
        int64_t tick = eventData[P_UPDATETICK].GetInt64();
        const Vector3& pos = eventData[P_POSITION].GetVector3();
        object->MoveTo(tick, pos);
    }
}

void WorldLevel::HandleObjectRotUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace AbEvents::ObjectRotUpdate;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    GameObject* object = objects_[objectId];
    if (object)
    {
        float rot = eventData[P_ROTATION].GetFloat();
        bool manual = eventData[P_MANUAL].GetBool();
        // Manual SetDirection -> don't update camera yaw because it comes from camera move
        object->SetYRotation(rot, !manual);
    }
}

void WorldLevel::HandleObjectStateUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace AbEvents::ObjectStateUpdate;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    GameObject* object = objects_[objectId];
    if (object)
    {
        int64_t tick = eventData[P_UPDATETICK].GetInt64();
        object->SetCreatureState(tick,
            static_cast<AB::GameProtocol::CreatureState>(eventData[P_STATE].GetInt()));
    }
}

void WorldLevel::HandleObjectSelected(StringHash eventType, VariantMap& eventData)
{
    using namespace AbEvents::ObjectSelected;
    uint32_t objectId = eventData[P_SOURCEID].GetUInt();
    uint32_t targetId = eventData[P_TARGETID].GetUInt();
    GameObject* object = objects_[objectId];
    if (object)
    {
        Actor* actor = dynamic_cast<Actor*>(object);
        if (actor)
        {
            if (targetId != 0)
            {
                SharedPtr<GameObject> target = objects_[targetId];
                if (target)
                {
                    actor->SetSelectedObject(target);
                    if (actor->objectType_ == ObjectTypeSelf)
                    {
                        targetWindow_->SetTarget(target);
                    }
                }
            }
            else
            {
                // Unselect
                actor->SetSelectedObject(SharedPtr<GameObject>());
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
    using namespace AbEvents::SetLevel;
    e[P_NAME] = "LoginLevel";
    SendEvent(AbEvents::E_SETLEVEL, e);
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
    if (!mailWindow_)
        mailWindow_ = new MailWindow(context_);
    FwClient* net = context_->GetSubsystem<FwClient>();
    net->GetMailHeaders();
    mailWindow_->visible_ = true;
}

void WorldLevel::HandleMenuPartyWindow(StringHash eventType, VariantMap& eventData)
{
    if (partyWindow_)
        partyWindow_->SetVisible(!partyWindow_->IsVisible());
}

void WorldLevel::HandleTargetWindowUnselectObject(StringHash eventType, VariantMap& eventData)
{
    player_->SelectObject(0);
}

Actor* WorldLevel::CreateActor(uint32_t id,
    const Vector3& position, const Vector3& scale, const Quaternion& direction,
    AB::GameProtocol::CreatureState state,
    PropReadStream& data)
{
    Actor* result = Actor::CreateActor(id, context_, scene_, position, direction, state, data);
    result->moveToPos_ = position;
    result->rotateTo_ = direction;
    result->GetNode()->SetScale(scale);
    return result;
}

void WorldLevel::CreatePlayer(uint32_t id,
    const Vector3& position, const Vector3& scale, const Quaternion& direction,
    AB::GameProtocol::CreatureState state,
    PropReadStream& data)
{
    player_ = Player::CreatePlayer(id, context_, scene_, position, direction, state, data);
    player_->moveToPos_ = position;
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

    ResourceCache* cache = GetSubsystem<ResourceCache>();

    chatWindow_ = uiRoot_->CreateChild<ChatWindow>();
    chatWindow_->SetAlignment(HA_LEFT, VA_BOTTOM);

    gameMenu_ = uiRoot_->CreateChild<GameMenu>();
    gameMenu_->SetAlignment(HA_LEFT, VA_TOP);
    SubscribeToEvent(gameMenu_, E_GAMEMENU_LOGOUT, URHO3D_HANDLER(WorldLevel, HandleMenuLogout));
    SubscribeToEvent(gameMenu_, E_GAMEMENU_SELECTCHAR, URHO3D_HANDLER(WorldLevel, HandleMenuSelectChar));
    SubscribeToEvent(gameMenu_, E_GAMEMENU_MAIL , URHO3D_HANDLER(WorldLevel, HandleMenuMail));
    SubscribeToEvent(gameMenu_, E_GAMEMENU_PARTYWINDOW, URHO3D_HANDLER(WorldLevel, HandleMenuPartyWindow));

    targetWindow_ = uiRoot_->CreateChild<TargetWindow>();
    targetWindow_->SetAlignment(HA_CENTER, VA_TOP);
    targetWindow_->SetVisible(false);
    SubscribeToEvent(targetWindow_, E_TARGETWINDOW_UNSELECT, URHO3D_HANDLER(WorldLevel, HandleTargetWindowUnselectObject));

    // Ping
    pingDot_ = uiRoot_->CreateChild<PingDot>();
    pingDot_->SetSize(IntVector2(24, 24));
    pingDot_->SetAlignment(HA_RIGHT, VA_BOTTOM);
}
