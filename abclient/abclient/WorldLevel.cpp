/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include "WorldLevel.h"
#include "ActorResourceBar.h"
#include "AudioManager.h"
#include "ChatWindow.h"
#include "CreditsWindow.h"
#include "DamageWindow.h"
#include "EffectsWindow.h"
#include "EquipmentWindow.h"
#include "FriendListWindow.h"
#include "FwClient.h"
#include "GameMenu.h"
#include "GameMessagesWindow.h"
#include "GuildWindow.h"
#include "InternalEvents.h"
#include "InventoryWindow.h"
#include "LevelManager.h"
#include "MailWindow.h"
#include "MapWindow.h"
#include "MathUtils.h"
#include "MissionMapWindow.h"
#include "NewMailWindow.h"
#include "Options.h"
#include "PartyWindow.h"
#include "ServerEvents.h"
#include "ShortcutEvents.h"
#include "Shortcuts.h"
#include "SkillBarWindow.h"
#include "SkillManager.h"
#include "SkillsWindow.h"
#include "TargetWindow.h"
#include "TimeUtils.h"
#include "TradeDialog.h"
#include "WindowManager.h"
#include <AB/ProtocolCodes.h>
#include <abshared/Mechanic.h>

//#define LOG_OBJECTSPAWN

WorldLevel::WorldLevel(Context* context) :
    BaseLevel(context),
    mailWindow_(nullptr),
    mapWindow_(nullptr),
    partyWindow_(nullptr)
{
}

WorldLevel::~WorldLevel()
{
    UnsubscribeFromAllEvents();
}

void WorldLevel::SubscribeToEvents()
{
    BaseLevel::SubscribeToEvents();

    SubscribeToEvent(Events::E_LEVELREADY, URHO3D_HANDLER(WorldLevel, HandleLevelReady));

    SubscribeToEvent(Events::E_SERVERJOINED, URHO3D_HANDLER(WorldLevel, HandleServerJoinedLeft));
    SubscribeToEvent(Events::E_SERVERLEFT, URHO3D_HANDLER(WorldLevel, HandleServerJoinedLeft));
    SubscribeToEvent(Events::E_OBJECTSPAWN, URHO3D_HANDLER(WorldLevel, HandleObjectSpawn));
    SubscribeToEvent(Events::E_OBJECTDESPAWN, URHO3D_HANDLER(WorldLevel, HandleObjectDespawn));
    SubscribeToEvent(Events::E_OBJECTPOSUPDATE, URHO3D_HANDLER(WorldLevel, HandleObjectPosUpdate));
    SubscribeToEvent(Events::E_OBJECTROTUPDATE, URHO3D_HANDLER(WorldLevel, HandleObjectRotUpdate));
    SubscribeToEvent(Events::E_OBJECTSETPOSITION, URHO3D_HANDLER(WorldLevel, HandleObjectSetPosition));
    SubscribeToEvent(Events::E_OBJECTSTATEUPDATE, URHO3D_HANDLER(WorldLevel, HandleObjectStateUpdate));
    SubscribeToEvent(Events::E_OBJECTSELECTED, URHO3D_HANDLER(WorldLevel, HandleObjectSelected));
    SubscribeToEvent(Events::E_OBJECTSPEEDUPDATE, URHO3D_HANDLER(WorldLevel, HandleObjectSpeedUpdate));
    SubscribeToEvent(Events::E_SKILLFAILURE, URHO3D_HANDLER(WorldLevel, HandleObjectSkillFailure));
    SubscribeToEvent(Events::E_ATTACKFAILURE, URHO3D_HANDLER(WorldLevel, HandleObjectAttackFailure));
    SubscribeToEvent(Events::E_PLAYERERROR, URHO3D_HANDLER(WorldLevel, HandlePlayerError));
    SubscribeToEvent(Events::E_PLAYERAUTORUN, URHO3D_HANDLER(WorldLevel, HandlePlayerAutorun));
    SubscribeToEvent(Events::E_OBJECTEFFECTADDED, URHO3D_HANDLER(WorldLevel, HandleObjectEffectAdded));
    SubscribeToEvent(Events::E_OBJECTEFFECTREMOVED, URHO3D_HANDLER(WorldLevel, HandleObjectEffectRemoved));
    SubscribeToEvent(Events::E_OBJECTRESOURCECHANGED, URHO3D_HANDLER(WorldLevel, HandleObjectResourceChange));
    SubscribeToEvent(Events::E_OBJECTITEMDROPPED, URHO3D_HANDLER(WorldLevel, HandleItemDropped));
    SubscribeToEvent(Events::E_DIALOGGTRIGGER, URHO3D_HANDLER(WorldLevel, HandleDialogTrigger));
    SubscribeToEvent(Events::E_SENDMAILTO, URHO3D_HANDLER(WorldLevel, HandleSendMailTo));
    SubscribeToEvent(Events::E_TRADEDIALOG_TRIGGER, URHO3D_HANDLER(WorldLevel, HandleTradeDialogTrigger));
    SubscribeToEvent(Events::E_TRADECANCEL, URHO3D_HANDLER(WorldLevel, HandleTradeCancel));
    SubscribeToEvent(Events::E_TRADEACCEPTED, URHO3D_HANDLER(WorldLevel, HandleTradeCancel));

    SubscribeToEvent(Events::E_SC_TOGGLEPARTYWINDOW, URHO3D_HANDLER(WorldLevel, HandleTogglePartyWindow));
    SubscribeToEvent(Events::E_SC_TOGGLEMISSIONMAPWINDOW, URHO3D_HANDLER(WorldLevel, HandleToggleMissionMapWindow));
    SubscribeToEvent(Events::E_SC_TOGGLEMAP, URHO3D_HANDLER(WorldLevel, HandleToggleMap));
    SubscribeToEvent(Events::E_SC_DEFAULTACTION, URHO3D_HANDLER(WorldLevel, HandleDefaultAction));
    SubscribeToEvent(Events::E_SC_TOGGLECHATWINDOW, URHO3D_HANDLER(WorldLevel, HandleToggleChatWindow));
    SubscribeToEvent(Events::E_SC_HIDEUI, URHO3D_HANDLER(WorldLevel, HandleHideUI));
    SubscribeToEvent(Events::E_SC_KEEPRUNNING, URHO3D_HANDLER(WorldLevel, HandleKeepRunning));
    SubscribeToEvent(Events::E_SC_LOGOUT, URHO3D_HANDLER(WorldLevel, HandleLogout));
    SubscribeToEvent(Events::E_SC_SELECTCHARACTER, URHO3D_HANDLER(WorldLevel, HandleSelectChar));
    SubscribeToEvent(Events::E_SC_TOGGLEMAILWINDOW, URHO3D_HANDLER(WorldLevel, HandleToggleMail));
    SubscribeToEvent(Events::E_SC_TOGGLENEWMAILWINDOW, URHO3D_HANDLER(WorldLevel, HandleToggleNewMail));
    SubscribeToEvent(Events::E_SC_REPLYMAIL, URHO3D_HANDLER(WorldLevel, HandleReplyMail));
    SubscribeToEvent(Events::E_SC_TOGGLEFRIENDLISTWINDOW, URHO3D_HANDLER(WorldLevel, HandleToggleFriendList));
    SubscribeToEvent(Events::E_SC_TOGGLEINVENTORYWINDOW, URHO3D_HANDLER(WorldLevel, HandleToggleInventoryWindow));
    SubscribeToEvent(Events::E_SC_TOGGLESKILLSWINDOW, URHO3D_HANDLER(WorldLevel, HandleToggleSkillsWindow));
    SubscribeToEvent(Events::E_SC_TOGGLEEQUIPWINDOW, URHO3D_HANDLER(WorldLevel, HandleToggleEquipWindow));
    SubscribeToEvent(Events::E_SC_TOGGLEGUILDWINDOW, URHO3D_HANDLER(WorldLevel, HandleToggleGuildWindow));
    SubscribeToEvent(Events::E_SC_SHOWCREDITS, URHO3D_HANDLER(WorldLevel, HandleShowCredits));
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(WorldLevel, HandleMouseDown));
    SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(WorldLevel, HandleMouseUp));
    SubscribeToEvent(E_MOUSEWHEEL, URHO3D_HANDLER(WorldLevel, HandleMouseWheel));
    SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(WorldLevel, HandleMouseMove));

    SubscribeToEvent(Events::E_SC_USESKILL1, URHO3D_HANDLER(WorldLevel, HandleUseSkill));
    SubscribeToEvent(Events::E_SC_USESKILL2, URHO3D_HANDLER(WorldLevel, HandleUseSkill));
    SubscribeToEvent(Events::E_SC_USESKILL3, URHO3D_HANDLER(WorldLevel, HandleUseSkill));
    SubscribeToEvent(Events::E_SC_USESKILL4, URHO3D_HANDLER(WorldLevel, HandleUseSkill));
    SubscribeToEvent(Events::E_SC_USESKILL5, URHO3D_HANDLER(WorldLevel, HandleUseSkill));
    SubscribeToEvent(Events::E_SC_USESKILL6, URHO3D_HANDLER(WorldLevel, HandleUseSkill));
    SubscribeToEvent(Events::E_SC_USESKILL7, URHO3D_HANDLER(WorldLevel, HandleUseSkill));
    SubscribeToEvent(Events::E_SC_USESKILL8, URHO3D_HANDLER(WorldLevel, HandleUseSkill));

    SubscribeToEvent(Events::E_SC_CANCEL, URHO3D_HANDLER(WorldLevel, HandleCancel));
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

void WorldLevel::RemoveUIWindows()
{
    gameMenu_->RemoveAllChildren();
    uiRoot_->RemoveChild(gameMenu_);
    inventoryWindow_->Clear();
    uiRoot_->RemoveChild(inventoryWindow_);
    friendsWindow_->Clear();
    uiRoot_->RemoveChild(friendsWindow_);
    uiRoot_->RemoveChild(guildWindow_);
}

void WorldLevel::HandleLevelReady(StringHash, VariantMap&)
{
    partyWindow_->SetPartySize(partySize_);
    partyWindow_->SetMode(AB::Entities::IsOutpost(mapType_) ?
        PartyWindowMode::ModeOutpost : PartyWindowMode::ModeGame);
}

void WorldLevel::HandleServerJoinedLeft(StringHash, VariantMap&)
{
    // Tell GameMenu to update servers
    VariantMap& eData = GetEventDataMap();
    SendEvent(Events::E_GOTSERVICES, eData);
}

void WorldLevel::HandleMouseDown(StringHash, VariantMap&)
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
        GameObject* object = GetObjectAt<GameObject>(input->GetMousePosition());
        if (object)
        {
            player_->ClickObject(object->gameId_);
            if (object->IsSelectable())
                player_->SelectObject(object->gameId_);
        }
        else
        {
            // If not object to pick check for mouse walking
            Options* o = GetSubsystem<Options>();
            if (!o->disableMouseWalking_)
            {
                Vector3 p;
                if (TerrainRaycast(input->GetMousePosition(), p))
                {
                    player_->GotoPosition(p);
                }
            }
        }
    }
}

void WorldLevel::HandleMouseUp(StringHash, VariantMap&)
{
    using namespace MouseButtonUp;
    if (rmbDown_)
    {
        Input* input = GetSubsystem<Input>();
        rmbDown_ = false;
        input->SetMousePosition(mouseDownPos_);
        input->SetMouseMode(MM_ABSOLUTE);
    }
}

void WorldLevel::HandleMouseWheel(StringHash, VariantMap& eventData)
{
    UI* ui = GetSubsystem<UI>();
    if (ui->GetFocusElement())
        return;

    using namespace MouseWheel;
    if (player_)
    {
        int delta = eventData[P_WHEEL].GetInt();
        player_->CameraZoom(delta < 0);
    }
}

bool WorldLevel::HoverObject(Actor* object)
{
    if (!object)
    {
        // Unhover
        if (auto* o = hoveredObject_.Get())
        {
            o->HoverEnd();
            hoveredObject_.Reset();
        }
        return true;
    }

    if (!object->selectable_)
        return false;

    if (auto* o = hoveredObject_.Get())
    {
        if (o == object)
            return true;

        o->HoverEnd();
    }
    object->HoverBegin();
    hoveredObject_ = object;
    return true;
}

void WorldLevel::HandleMouseMove(StringHash, VariantMap&)
{
    if (!viewport_)
        // Not ready yet
        return;

    // Hover object only when no mouse button is pressed
    Input* input = GetSubsystem<Input>();
    if (input->GetMouseButtonDown(MOUSEB_LEFT) || input->GetMouseButtonDown(MOUSEB_RIGHT))
        return;

    Actor* object = GetObjectAt<Actor>(input->GetMousePosition());
    HoverObject(object);
}

void WorldLevel::Update(StringHash, VariantMap&)
{
    using namespace Update;

    if (!player_)
        return;

    Shortcuts* sc = GetSubsystem<Shortcuts>();
    Options* op = GetSubsystem<Options>();

    // Clear previous controls
    player_->controls_.Set(CTRL_MOVE_FORWARD | CTRL_MOVE_BACK | CTRL_MOVE_LEFT | CTRL_MOVE_RIGHT |
        CTRL_TURN_LEFT | CTRL_TURN_RIGHT, false);

    // Update controls using keys
    if (sc->Test(Events::E_SC_MOVEFORWARD))
    {
        player_->controls_.Set(CTRL_MOVE_FORWARD, true);
        if (!sc->Test(Events::E_SC_KEEPRUNNING))
            player_->controls_.Set(CTRL_MOVE_LOCK, false);
    }
    if (sc->Test(Events::E_SC_MOVEBACKWARD))
    {
        player_->controls_.Set(CTRL_MOVE_BACK, true);
        if (!sc->Test(Events::E_SC_KEEPRUNNING))
            player_->controls_.Set(CTRL_MOVE_LOCK, false);
    }
    player_->controls_.Set(CTRL_MOVE_LEFT, sc->Test(Events::E_SC_MOVELEFT));
    player_->controls_.Set(CTRL_MOVE_RIGHT, sc->Test(Events::E_SC_MOVERIGHT));

    if (sc->Test(Events::E_SC_MOUSELOOK))
    {
        player_->controls_.Set(CTRL_MOVE_LEFT, player_->controls_.IsDown(CTRL_MOVE_LEFT) ||
            sc->Test(Events::E_SC_TURNLEFT));
        player_->controls_.Set(CTRL_MOVE_RIGHT, player_->controls_.IsDown(CTRL_MOVE_RIGHT) ||
            sc->Test(Events::E_SC_TURNRIGHT));

        Input* input = GetSubsystem<Input>();
        player_->controls_.yaw_ += static_cast<float>(input->GetMouseMoveX()) * op->mouseSensitivity_;
        player_->controls_.pitch_ += static_cast<float>(input->GetMouseMoveY()) * op->mouseSensitivity_;
    }
    else
    {
        player_->controls_.Set(CTRL_TURN_LEFT, sc->Test(Events::E_SC_TURNLEFT));
        player_->controls_.Set(CTRL_TURN_RIGHT, sc->Test(Events::E_SC_TURNRIGHT));
    }

    // Limit pitch
    player_->controls_.pitch_ = Clamp(player_->controls_.pitch_, -80.0f, 80.0f);
    NormalizeAngle(player_->controls_.yaw_);
}

void WorldLevel::SetupViewport()
{
    BaseLevel::SetupViewport();
    postProcess_->SetUseBloomHDR(true);
    postProcess_->SetUseColorCorrection(true);
//    postProcess_->SetUseGammaCorrection(true);
//    postProcess_->SetAutoExposureLumRange(Vector2(0.5f, 1.0f));
//    postProcess_->SetAutoExposureAdaptRate(0.5f);
//    postProcess_->SetAutoExposureMidGrey(1.0f);
//    postProcess_->SetUseAutoExposure(true);

    if (equipWindow_ && equipWindow_->IsVisible())
        equipWindow_->Initialize(*postProcess_);
}

void WorldLevel::CreateScene()
{
    BaseLevel::CreateScene();
    NavigationMesh* navMesh = scene_->GetComponent<NavigationMesh>();
    if (navMesh)
        navMesh->Build();

    using namespace Events::AudioPlayMapMusic;
    VariantMap& e = GetEventDataMap();
    e[P_MAPUUID] = mapUuid_;
    SendEvent(Events::E_AUDIOPLAYMAPMUSIC, e);
}

void WorldLevel::PostUpdate(StringHash eventType, VariantMap& eventData)
{
    BaseLevel::PostUpdate(eventType, eventData);
}

void WorldLevel::HandleObjectSpawn(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectSpawn;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    if (objects_.Contains(objectId))
        return;
    AB::GameProtocol::GameObjectType type = static_cast<AB::GameProtocol::GameObjectType>(eventData[P_OBJECTTYPE].GetUInt());
    int64_t tick = eventData[P_UPDATETICK].GetInt64();
    const Vector3& pos = eventData[P_POSITION].GetVector3();
    float rot = eventData[P_ROTATION].GetFloat();
    Quaternion direction;
    float deg = -rot * (180.0f / static_cast<float>(M_PI));
    direction.FromAngleAxis(deg, Vector3(0.0f, 1.0f, 0.0f));
    const Vector3& scale = eventData[P_SCALE].GetVector3();
    AB::GameProtocol::CreatureState state = static_cast<AB::GameProtocol::CreatureState>(eventData[P_STATE].GetUInt());
    float speed = eventData[P_SPEEDFACTOR].GetFloat();
    uint32_t groupId = eventData[P_GROUPID].GetUInt();
    uint8_t groupPos = static_cast<uint8_t>(eventData[P_GROUPPOS].GetUInt());
    uint32_t groupMask = eventData[P_GROUPMASK].GetUInt();
    const String& d = eventData[P_DATA].GetString();
    bool existing = eventData[P_EXISTING].GetBool();
    bool undestroyable = eventData[P_UNDESTROYABLE].GetBool();
    bool selectable = eventData[P_SELECTABLE].GetBool();
    sa::PropReadStream data(d.CString(), d.Length());
    SpawnObject(tick, objectId, type, existing, pos, scale, direction,
        undestroyable, selectable,
        state, speed, groupId, groupPos, groupMask, data);
}

void WorldLevel::SpawnObject(int64_t updateTick, uint32_t id, AB::GameProtocol::GameObjectType objectType, bool existing,
    const Vector3& position, const Vector3& scale, const Quaternion& rot,
    bool undestroyable, bool selectable, AB::GameProtocol::CreatureState state, float speed,
    uint32_t groupId, uint8_t groupPos, uint32_t groupMask,
    sa::PropReadStream& data)
{
    FwClient* client = GetSubsystem<FwClient>();
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
    case AB::GameProtocol::ObjectTypeAreaOfEffect:
        object = CreateActor(id, position, scale, rot, state, data);
        object->objectType_ = ObjectTypeAreaOfEffect;
        break;
    case AB::GameProtocol::ObjectTypeItemDrop:
        object = CreateActor(id, position, scale, rot, state, data);
        object->objectType_ = ObjectTypeItemDrop;
        break;
    case AB::GameProtocol::ObjectTypeProjectile:
        object = CreateActor(id, position, scale, rot, state, data);
        object->objectType_ = ObjectTypeProjectile;
        break;
    case AB::GameProtocol::ObjectTypeUnknown:
    case AB::GameProtocol::ObjectTypeTerrainPatch:
    case AB::GameProtocol::ObjectTypeStatic:
    case AB::GameProtocol::ObjectTypeSentToPlayer:
        // Not for us
        break;
    }
    if (object)
    {
        object->spawnTickServer_ = updateTick;
        object->groupId_ = groupId;
        object->groupPos_ = groupPos;
        if (groupMask != 0)
            object->groupMask_ = groupMask;

        const float p[3] = { position.x_, position.y_, position.z_ };
        // Here an object is always an Actor
        Actor* actor = To<Actor>(object);
        actor->posExtrapolator_.Reset(object->GetServerTime(updateTick),
            object->GetClientTime(), p);
        object->GetNode()->SetName(actor->name_);
        object->undestroyable_ = undestroyable;
        object->selectable_ = selectable;
        object->SetSpeedFactor(updateTick, speed);
        objects_[id] = object;
        nodeIds_[object->GetNode()->GetID()] = id;
        actor->AddActorUI();

#ifdef LOG_OBJECTSPAWN
        URHO3D_LOGINFOF("Spawned object %d: %s", actor->gameId_, actor->name_.CString());
#endif
        switch (object->objectType_)
        {
        case ObjectTypePlayer:
            if (!existing)
                chatWindow_->AddLine(static_cast<Actor*>(object)->name_ + " joined the game", "ChatLogServerInfoText");
            break;
        case ObjectTypeSelf:
        {
            To<Player>(object)->UpdateMumbleContext();
            To<Player>(object)->UpdateUI();
            break;
        }
        default:
            break;
        }

        // Update party window
        partyWindow_->OnObjectSpawned(object, groupId, groupPos);
    }
}

void WorldLevel::HandleObjectDespawn(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectDespawn;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    GameObject* object = objects_[objectId];
    if (object)
    {
#ifdef LOG_OBJECTSPAWN
        URHO3D_LOGINFOF("Despawn object %d", object->gameId_);
#endif
        GameObject* selO = player_->GetSelectedObject();
        if (selO && selO->gameId_ == object->gameId_)
        {
            // If the selected object leaves unselect it
            player_->SetSelectedObject(SharedPtr<GameObject>());
            targetWindow_->SetTarget(SharedPtr<Actor>());
        }
        object->RemoveFromScene();
        object->GetNode()->Remove();
        if (object->objectType_ == ObjectTypePlayer)
        {
            Actor* act = To<Actor>(object);
            chatWindow_->AddLine(act->name_ + " left the game", "ChatLogServerInfoText");
        }
        objects_.Erase(objectId);
    }
}

void WorldLevel::HandleObjectPosUpdate(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectPosUpdate;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    GameObject* object = objects_[objectId];
    if (object)
    {
        int64_t tick = eventData[P_UPDATETICK].GetInt64();
        const Vector3& pos = eventData[P_POSITION].GetVector3();
        object->MoveTo(tick, pos);
    }
}

void WorldLevel::HandleObjectRotUpdate(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectRotUpdate;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    GameObject* object = objects_[objectId];
    if (object)
    {
        float rot = eventData[P_ROTATION].GetFloat();
        bool manual = eventData[P_MANUAL].GetBool();
        int64_t tick = eventData[P_UPDATETICK].GetInt64();
        // Manual SetDirection -> don't update camera yaw because it comes from camera move
        object->SetYRotation(tick, rot, !manual);
    }
}

void WorldLevel::HandleObjectSetPosition(StringHash, VariantMap& eventData)
{
    // Force update position
    using namespace Events::ObjectSetPosition;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    GameObject* object = objects_[objectId];
    if (object)
    {
        const Vector3& pos = eventData[P_POSITION].GetVector3();
        int64_t tick = eventData[P_UPDATETICK].GetInt64();
        object->ForcePosition(tick, pos);
    }
}

void WorldLevel::HandleObjectStateUpdate(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectStateUpdate;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    GameObject* object = objects_[objectId];
    if (object)
    {
        int64_t tick = eventData[P_UPDATETICK].GetInt64();
        object->SetCreatureState(tick,
            static_cast<AB::GameProtocol::CreatureState>(eventData[P_STATE].GetInt()));
    }
}

void WorldLevel::HandleObjectSpeedUpdate(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectSpeedUpdate;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    GameObject* object = objects_[objectId];
    if (object)
    {
        int64_t tick = eventData[P_UPDATETICK].GetInt64();
        object->SetSpeedFactor(tick, eventData[P_SPEEDFACTOR].GetFloat());
    }
}

void WorldLevel::HandleObjectSelected(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectSelected;
    uint32_t objectId = eventData[P_SOURCEID].GetUInt();
    uint32_t targetId = eventData[P_TARGETID].GetUInt();
    GameObject* object = objects_[objectId];
    if (object)
    {
        Actor* actor = To<Actor>(object);
        if (actor)
        {
            if (targetId != 0)
            {
                GameObject* target = objects_[targetId];
                if (target)
                {
                    actor->SetSelectedObject(SharedPtr<GameObject>(target));
                    if (actor->objectType_ == ObjectTypeSelf)
                    {
                        targetWindow_->SetTarget(SharedPtr<Actor>(To<Actor>(target)));
                        partyWindow_->SelectItem(targetId);
                    }
                }
            }
            else
            {
                // Unselect
                actor->SetSelectedObject(SharedPtr<GameObject>());
                if (actor->objectType_ == ObjectTypeSelf)
                {
                    targetWindow_->SetTarget(SharedPtr<Actor>());
                    partyWindow_->UnselectItem(targetId);
                }
            }
        }
    }
}

void WorldLevel::HandleObjectSkillFailure(StringHash, VariantMap& eventData)
{
    using namespace Events::SkillFailure;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    GameObject* object = objects_[objectId];
    if (object)
    {
        AB::GameProtocol::SkillError error = static_cast<AB::GameProtocol::SkillError>(eventData[P_ERROR].GetUInt());
        object->OnSkillError(error);
        if (object->objectType_ == ObjectTypeSelf)
        {
            const String& msg = eventData[P_ERRORMSG].GetString();
            if (!msg.Empty())
            {
                WindowManager* wm = GetSubsystem<WindowManager>();
                GameMessagesWindow* wnd = dynamic_cast<GameMessagesWindow*>(wm->GetWindow(WINDOW_GAMEMESSAGES, true).Get());
                wnd->ShowError(msg);
            }
        }
    }
}

void WorldLevel::HandleObjectAttackFailure(StringHash, VariantMap& eventData)
{
    using namespace Events::AttackFailure;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    GameObject* object = objects_[objectId];
    if (object)
    {
        AB::GameProtocol::AttackError error = static_cast<AB::GameProtocol::AttackError>(eventData[P_ERROR].GetUInt());
        object->OnAttackError(error);
        if (object->objectType_ == ObjectTypeSelf)
        {
            const String& msg = eventData[P_ERRORMSG].GetString();
            if (!msg.Empty())
            {
                WindowManager* wm = GetSubsystem<WindowManager>();
                GameMessagesWindow* wnd = dynamic_cast<GameMessagesWindow*>(wm->GetWindow(WINDOW_GAMEMESSAGES, true).Get());
                wnd->ShowError(msg);
            }
        }
    }
}

void WorldLevel::HandlePlayerError(StringHash, VariantMap& eventData)
{
    using namespace Events::PlayerError;
    AB::GameProtocol::PlayerErrorValue error = static_cast<AB::GameProtocol::PlayerErrorValue>(eventData[P_ERROR].GetUInt());
    if (error == AB::GameProtocol::PlayerErrorNone)
        return;
    const String& msg = eventData[P_ERRORMSG].GetString();
    if (!msg.Empty())
    {
        WindowManager* wm = GetSubsystem<WindowManager>();
        GameMessagesWindow* wnd = dynamic_cast<GameMessagesWindow*>(wm->GetWindow(WINDOW_GAMEMESSAGES, true).Get());
        wnd->ShowError(msg);
    }
}

void WorldLevel::HandlePlayerAutorun(StringHash, VariantMap& eventData)
{
    using namespace Events::PlayerAutorun;
    bool autorun = eventData[P_AUTORUN].GetBool();
    if (player_)
        player_->autoRun_ = autorun;
}

void WorldLevel::HandleObjectEffectAdded(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectEffectAdded;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    GameObject* object = objects_[objectId];
    if (object)
    {
        uint32_t effectIndex = eventData[P_EFFECTINDEX].GetUInt();
        uint32_t ticks = eventData[P_TICKS].GetUInt();
        object->OnEffectAdded(effectIndex, ticks);
        if (object->objectType_ == ObjectTypeSelf)
        {
            WindowManager* wm = GetSubsystem<WindowManager>();
            EffectsWindow* wnd = dynamic_cast<EffectsWindow*>(wm->GetWindow(WINDOW_EFFECTS, true).Get());
            wnd->SetVisible(true);
            wnd->EffectAdded(effectIndex, ticks);
        }
    }
}

void WorldLevel::HandleObjectEffectRemoved(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectEffectRemoved;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    GameObject* object = objects_[objectId];
    if (object)
    {
        uint32_t effectIndex = eventData[P_EFFECTINDEX].GetUInt();
        object->OnEffectRemoved(effectIndex);
        if (object->objectType_ == ObjectTypeSelf)
        {
            WindowManager* wm = GetSubsystem<WindowManager>();
            EffectsWindow* wnd = dynamic_cast<EffectsWindow*>(wm->GetWindow(WINDOW_EFFECTS, true).Get());
            wnd->EffectRemoved(effectIndex);
        }
    }
}

void WorldLevel::HandleObjectResourceChange(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectResourceChanged;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    AB::GameProtocol::ResourceType resType = static_cast<AB::GameProtocol::ResourceType>(eventData[P_RESTYPE].GetUInt());
    int32_t value = eventData[P_VALUE].GetInt();
    GameObject* object = objects_[objectId];
    if (Is<Actor>(object))
        To<Actor>(object)->ChangeResource(resType, value);
}

void WorldLevel::HandleLogout(StringHash, VariantMap&)
{
    RemoveUIWindows();
    FwClient* net = GetSubsystem<FwClient>();
    net->PartyLeave();
    net->Logout();
    VariantMap& e = GetEventDataMap();
    using namespace Events::SetLevel;
    e[P_NAME] = "LoginLevel";
    SendEvent(Events::E_SETLEVEL, e);
}

void WorldLevel::HandleSelectChar(StringHash, VariantMap&)
{
    RemoveUIWindows();
    FwClient* net = GetSubsystem<FwClient>();
    net->PartyLeave();
    net->Logout();
    net->Login(net->accountName_, net->accountPass_);
}

void WorldLevel::HandleTogglePartyWindow(StringHash, VariantMap&)
{
    if (partyWindow_)
    {
        partyWindow_->SetVisible(!partyWindow_->IsVisible());
        if (partyWindow_->IsVisible())
            partyWindow_->BringToFront();
    }
}

void WorldLevel::HandleToggleInventoryWindow(StringHash, VariantMap&)
{
    inventoryWindow_->SetVisible(!inventoryWindow_->IsVisible());
    if (inventoryWindow_->IsVisible())
    {
        inventoryWindow_->BringToFront();
        inventoryWindow_->GetInventory();
    }
}

void WorldLevel::HandleToggleSkillsWindow(StringHash, VariantMap&)
{
    WindowManager* wm = GetSubsystem<WindowManager>();
    SharedPtr<SkillsWindow> wnd;
    wnd.StaticCast(wm->GetWindow(WINDOW_SKILLS, true));
    wnd->SetVisible(!wnd->IsVisible());
    if (wnd->IsVisible())
    {
        wnd->BringToFront();
        wnd->UpdateAll();
    }
}

void WorldLevel::HandleToggleEquipWindow(StringHash, VariantMap&)
{
    equipWindow_->SetVisible(!equipWindow_->IsVisible());
    if (equipWindow_->IsVisible())
    {
        equipWindow_->Initialize(*postProcess_);
        equipWindow_->BringToFront();
    }
}

void WorldLevel::HandleToggleMissionMapWindow(StringHash, VariantMap&)
{
    if (missionMap_)
    {
        missionMap_->SetVisible(!missionMap_->IsVisible());
        if (missionMap_->IsVisible())
            missionMap_->BringToFront();
    }
}

void WorldLevel::HandleTargetWindowUnselectObject(StringHash, VariantMap&)
{
    player_->SelectObject(0);
}

void WorldLevel::HandleToggleMap(StringHash, VariantMap&)
{
    ToggleMap();
}

void WorldLevel::HandleHideUI(StringHash, VariantMap&)
{
    uiRoot_->SetVisible(!uiRoot_->IsVisible());
}

void WorldLevel::HandleDefaultAction(StringHash, VariantMap&)
{
    auto sel = player_->GetSelectedObject();
    if (!Is<Actor>(sel))
        return;

    if (!AB::Entities::IsOutpost(mapType_) && player_->IsEnemy(To<Actor>(sel)))
        player_->Attack();
    else
        player_->FollowSelected();
}

void WorldLevel::HandleKeepRunning(StringHash, VariantMap&)
{
    player_->controls_.Set(CTRL_MOVE_LOCK, !player_->controls_.IsDown(CTRL_MOVE_LOCK));
}

void WorldLevel::HandleToggleChatWindow(StringHash, VariantMap&)
{
    chatWindow_->SetVisible(!chatWindow_->IsVisible());
}

void WorldLevel::HandleToggleMail(StringHash, VariantMap&)
{
    WindowManager* wm = GetSubsystem<WindowManager>();
    MailWindow* wnd = dynamic_cast<MailWindow*>(wm->GetWindow(WINDOW_MAIL, true).Get());
    wnd->SetVisible(!wnd->IsVisible());
    if (wnd->IsVisible())
    {
        wnd->BringToFront();
        wnd->GetHeaders();
    }
}

void WorldLevel::HandleReplyMail(StringHash, VariantMap& eventData)
{
    WindowManager* wm = GetSubsystem<WindowManager>();
    NewMailWindow* wnd = dynamic_cast<NewMailWindow*>(wm->GetWindow(WINDOW_NEWMAIL, true).Get());
    using namespace Events::ReplyMail;
    wnd->SetRecipient(eventData[P_RECIPIENT].GetString());
    const String& subj = eventData[P_SUBJECT].GetString();
    if (!subj.StartsWith("Re: "))
        wnd->SetSubject("Re: " + subj);
    else
        wnd->SetSubject(subj);

    const String& body = eventData[P_BODY].GetString();
    if (!body.Empty())
    {
        Vector<String> bodyLines = body.Split('\n', true);
        String newBody = "\n\n";
        for (auto& l : bodyLines)
            newBody += "> " + l + "\n";
        wnd->SetBody(newBody);
    }

    wnd->SetVisible(true);
    wnd->BringToFront();
    wnd->FocusBody();
}

void WorldLevel::HandleSendMailTo(StringHash, VariantMap& eventData)
{
    WindowManager* wm = GetSubsystem<WindowManager>();
    NewMailWindow* wnd = dynamic_cast<NewMailWindow*>(wm->GetWindow(WINDOW_NEWMAIL, true).Get());
    using namespace Events::SendMailTo;
    wnd->SetRecipient(eventData[P_NAME].GetString());
    wnd->SetVisible(true);
    wnd->BringToFront();
}

void WorldLevel::HandleToggleNewMail(StringHash, VariantMap&)
{
    WindowManager* wm = GetSubsystem<WindowManager>();
    NewMailWindow* wnd = dynamic_cast<NewMailWindow*>(wm->GetWindow(WINDOW_NEWMAIL, true).Get());
    wnd->SetVisible(!wnd->IsVisible());
    if (wnd->IsVisible())
    {
        wnd->BringToFront();
        wnd->FocusRecipient();
    }
}

void WorldLevel::HandleToggleFriendList(StringHash, VariantMap&)
{
    friendsWindow_->SetVisible(!friendsWindow_->IsVisible());
    if (friendsWindow_->IsVisible())
    {
        friendsWindow_->BringToFront();
        friendsWindow_->GetList();
    }
}

void WorldLevel::HandleToggleGuildWindow(StringHash, VariantMap&)
{
    guildWindow_->SetVisible(!guildWindow_->IsVisible());
    if (guildWindow_->IsVisible())
    {
        guildWindow_->BringToFront();
        guildWindow_->UpdateAll();
    }
}

void WorldLevel::HandleShowCredits(StringHash, VariantMap&)
{
    auto* ui = GetSubsystem<UI>();
    CreditsWindow* wnd = ui->GetRoot()->GetChildDynamicCast<CreditsWindow>(CreditsWindow::NAME);
    if (wnd)
        ui->GetRoot()->RemoveChild(wnd);
    wnd = ui->GetRoot()->CreateChild<CreditsWindow>(CreditsWindow::NAME);
    wnd->SetVisible(true);
}

void WorldLevel::HandleUseSkill(StringHash eventType, VariantMap&)
{
    FwClient* client = GetSubsystem<FwClient>();
    uint32_t index = 0;
    if (eventType == Events::E_SC_USESKILL1)
        index = 1;
    else if (eventType == Events::E_SC_USESKILL2)
        index = 2;
    else if (eventType == Events::E_SC_USESKILL3)
        index = 3;
    else if (eventType == Events::E_SC_USESKILL4)
        index = 4;
    else if (eventType == Events::E_SC_USESKILL5)
        index = 5;
    else if (eventType == Events::E_SC_USESKILL6)
        index = 6;
    else if (eventType == Events::E_SC_USESKILL7)
        index = 7;
    else if (eventType == Events::E_SC_USESKILL8)
        index = 8;
    else
        return;
    client->UseSkill(index);
}

void WorldLevel::HandleCancel(StringHash, VariantMap&)
{
    FwClient* client = GetSubsystem<FwClient>();
    client->Cancel();
}

void WorldLevel::HandleItemDropped(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectItemDropped;
    uint32_t itemId = eventData[P_ITEMID].GetUInt();
    uint32_t count = eventData[P_COUNT].GetUInt();
    uint32_t value = eventData[P_VALUE].GetUInt();
    GameObject* item = GetObject<GameObject>(itemId);
    if (item)
    {
        item->count_ = count;
        item->value_ = value;
    }
}

void WorldLevel::HandleDialogTrigger(StringHash, VariantMap& eventData)
{
    using namespace Events::DialogTrigger;

    uint32_t id = eventData[P_TIGGERERID].GetUInt();
    if (id != 0)
    {
        auto* actor = GetObject<Actor>(id);
        if (!actor)
            return;
        float dist = player_->GetNode()->GetPosition().DistanceToPoint(actor->GetNode()->GetPosition());
        if (dist > Game::RANGE_PICK_UP)
            return;
    }
    AB::Dialogs dialog = static_cast<AB::Dialogs>(eventData[P_DIALOGID].GetUInt());
    WindowManager* wm = GetSubsystem<WindowManager>();
    DialogWindow* wnd = wm->GetDialog(dialog, true);
    if (wnd)
    {
        wnd->Initialize();
        wnd->SetVisible(true);
        wnd->BringToFront();
    }
}

void WorldLevel::HandleTradeDialogTrigger(StringHash, VariantMap& eventData)
{
    using namespace Events::TradeDialogTrigger;
    uint32_t sourceId = eventData[P_SOURDEID].GetUInt();
    uint32_t targetId = eventData[P_TARGETID].GetUInt();
    if (sourceId == 0 || targetId == 0)
        return;
    if (sourceId == targetId)
        return;
    // That's we. If we are not a player, something is wrong.
    auto* source = GetObject<Actor>(sourceId);
    if (!source)
        return;
    if (source->objectType_ != ObjectTypePlayer && source->objectType_ != ObjectTypeSelf)
        return;
    auto* target = GetObject<Actor>(targetId);
    if (!target)
        return;
    if (target->objectType_ != ObjectTypePlayer && target->objectType_ != ObjectTypeSelf)
        return;

    SharedPtr<Player> player;
    SharedPtr<Actor> partner;
    if (source->objectType_ == ObjectTypeSelf)
    {
        player.StaticCast(SharedPtr<Actor>(source));
        partner = target;
    }
    else if (target->objectType_ == ObjectTypeSelf)
    {
        player.StaticCast(SharedPtr<Actor>(target));
        partner = source;
    }
    else
        return;

    tradeDialog_ = MakeShared<TradeDialog>(context_, player, partner);
}

void WorldLevel::HandleTradeCancel(StringHash, VariantMap&)
{
    if (tradeDialog_)
        tradeDialog_->Close();
}

Actor* WorldLevel::CreateActor(uint32_t id,
    const Vector3& position, const Vector3& scale, const Quaternion& direction,
    AB::GameProtocol::CreatureState state,
    sa::PropReadStream& data)
{
    Actor* result = Actor::CreateActor(id, scene_, position, direction, state, data);
    result->moveToPos_ = position;
    result->rotateTo_ = direction;
    result->GetNode()->SetScale(scale);
    return result;
}

Actor* WorldLevel::GetActorByName(const String& name, ObjectType type /* = ObjectTypePlayer */)
{
    for (const auto& o : objects_)
    {
        if (o.second_->objectType_ == type)
        {
            Actor* a = To<Actor>(o.second_.Get());
            if (a && a->name_.Compare(name, false) == 0)
                return a;
        }
    }
    return nullptr;
}

TradeDialog* WorldLevel::GetTradeDialog() const
{
    if (tradeDialog_)
        return tradeDialog_.Get();
    return nullptr;
}

void WorldLevel::CreatePlayer(uint32_t id,
    const Vector3& position, const Vector3& scale, const Quaternion& direction,
    AB::GameProtocol::CreatureState state,
    sa::PropReadStream& data)
{
    player_ = Player::CreatePlayer(id, scene_);
    skillBar_->SetActor(player_);
    player_->moveToPos_ = position;
    player_->rotateTo_ = direction;
    player_->GetNode()->SetScale(scale);
    player_->UpdateYaw();
    player_->Unserialize(data);
    player_->Init(scene_, position, direction, state);
    player_->PlayAnimation(ANIM_IDLE, true, 0.0f);

    cameraNode_ = player_->cameraNode_;
    // Add sound listener to camera node, also Guild Wars does it so.
    Node* listenerNode = cameraNode_->CreateChild("SoundListenerNode");
    // Let's face the sound
    listenerNode->SetDirection(Vector3(0.0f, M_HALF_PI, 0.0f));
    SoundListener* soundListener = listenerNode->CreateComponent<SoundListener>();
    GetSubsystem<Audio>()->SetListener(soundListener);
    SetupViewport();
    partyWindow_->SetPlayer(player_);
    chatWindow_->SayHello(player_);
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

    WindowManager* wm = GetSubsystem<WindowManager>();

    Options* op = GetSubsystem<Options>();
    gameMenu_ = SharedPtr<GameMenu>(new GameMenu(context_));
    op->LoadWindow(gameMenu_);
    gameMenu_->SetVisible(true);
    uiRoot_->AddChild(gameMenu_);

    // Always add these windows, create if not yet done
    chatWindow_.DynamicCast(wm->GetWindow(WINDOW_CHAT));
    uiRoot_->AddChild(chatWindow_);

    skillBar_.DynamicCast(wm->GetWindow(WINDOW_SKILLBAR));
    uiRoot_->AddChild(skillBar_);
    healthBar_.DynamicCast(wm->GetWindow(WINDOW_HEALTHBAR));
    healthBar_->SetActor(player_);
    uiRoot_->AddChild(healthBar_);
    energyBar_.DynamicCast(wm->GetWindow(WINDOW_ENERGYBAR));
    energyBar_->SetActor(player_);
    uiRoot_->AddChild(energyBar_);
    damageWindow_.DynamicCast(wm->GetWindow(WINDOW_DAMAGE));
    damageWindow_->Clear();
    uiRoot_->AddChild(damageWindow_);

    effectsWindow_.DynamicCast(wm->GetWindow(WINDOW_EFFECTS));
    effectsWindow_->Clear();
    uiRoot_->AddChild(effectsWindow_);

    targetWindow_.DynamicCast(wm->GetWindow(WINDOW_TARGET));
    targetWindow_->SetVisible(false);
    uiRoot_->AddChild(targetWindow_);
    SubscribeToEvent(targetWindow_, E_TARGETWINDOW_UNSELECT, URHO3D_HANDLER(WorldLevel, HandleTargetWindowUnselectObject));

    pingDot_.DynamicCast(wm->GetWindow(WINDOW_PINGDOT));
    uiRoot_->AddChild(pingDot_);

    missionMap_.DynamicCast(wm->GetWindow(WINDOW_MISSIONMAP));
    uiRoot_->AddChild(missionMap_);
    missionMap_->SetScene(scene_);

    inventoryWindow_.DynamicCast(wm->GetWindow(WINDOW_INVENTORY));
    uiRoot_->AddChild(inventoryWindow_);
    if (inventoryWindow_->IsVisible())
        inventoryWindow_->GetInventory();
    equipWindow_.DynamicCast(wm->GetWindow(WINDOW_EQUIPMENT));
    uiRoot_->AddChild(equipWindow_);
    friendsWindow_.DynamicCast(wm->GetWindow(WINDOW_FRIENDLIST));
    uiRoot_->AddChild(friendsWindow_);
    if (friendsWindow_->IsVisible())
        friendsWindow_->GetList();

    guildWindow_.DynamicCast(wm->GetWindow(WINDOW_GUILD));
    uiRoot_->AddChild(guildWindow_);
    if (guildWindow_->IsVisible())
        guildWindow_->UpdateAll();
}
