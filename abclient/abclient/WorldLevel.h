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

#pragma once

#include "BaseLevel.h"
#include "GameObject.h"
#include <stdint.h>
#include <sa/PropStream.h>
#include "Actor.h"
#include "PingDot.h"
#include <AB/Entities/Game.h>

class ActorEnergyBar;
class ActorHealthBar;
class ChatWindow;
class DamageWindow;
class EffectsWindow;
class EquipmentWindow;
class FriendListWindow;
class GameMenu;
class GuildWindow;
class InventoryWindow;
class MailWindow;
class MapWindow;
class MissionMapWindow;
class PartyWindow;
class SkillBarWindow;
class TargetWindow;
class TradeDialog;

struct SpawnObjectStuct
{
    uint32_t id;
    AB::GameProtocol::GameObjectType type;
    int64_t tick;
    Vector3 position;
    Quaternion direction;
    Vector3 scale;
    AB::GameProtocol::CreatureState state;
    float speed;
    uint32_t groupId;
    uint8_t groupPos;
    uint32_t groupMask;
    bool isExisting;
    bool isUndestroyable;
    bool isSelectable;
    sa::PropReadStream data;
};

/// All World maps, Outposts, Combat, Exploreable...
/// These all have the Game UI, though the UI may slightly differ, e.g. the Party window.
class WorldLevel : public BaseLevel
{
    URHO3D_OBJECT(WorldLevel, BaseLevel)
public:
    WorldLevel(Context* context);
    ~WorldLevel() override;
    void CreatePlayer(uint32_t id,
        const Vector3& position, const Vector3& scale, const Quaternion& direction,
        AB::GameProtocol::CreatureState state,
        sa::PropReadStream& data);
    Actor* CreateActor(uint32_t id,
        const Vector3& position, const Vector3& scale, const Quaternion& direction,
        AB::GameProtocol::CreatureState state,
        sa::PropReadStream& data);
    template<typename T>
    T* GetObject(uint32_t id)
    {
        if (objects_.Contains(id))
        {
            if (Is<T>(objects_[id].Get()))
                return To<T>(objects_[id].Get());
        }
        return nullptr;
    }
    Actor* GetActorByName(const String& name, ObjectType type = ObjectType::Player) const;
    /// Local Node IDs are not the same as Object IDs on the server.
    GameObject* GetObjectFromNode(Node* node)
    {
        unsigned id = node->GetID();
        uint32_t objectId = nodeIds_[id];
        if (objectId == 0 && node->GetParent())
        {
            id = node->GetParent()->GetID();
            objectId = nodeIds_[id];
        }
        if (objectId != 0)
            return objects_[objectId].Get();
        return nullptr;
    }
    bool HoverObject(Actor* object);
    const Actor* GetHoveredObject() const { return hoveredObject_.Get(); }
    TradeDialog* GetTradeDialog() const;
protected:
    SharedPtr<ChatWindow> chatWindow_;
    SharedPtr<PingDot> pingDot_;
    SharedPtr<GameMenu> gameMenu_;
    SharedPtr<TargetWindow> targetWindow_;
    SharedPtr<MailWindow> mailWindow_;
    SharedPtr<MapWindow> mapWindow_;
    SharedPtr<PartyWindow> partyWindow_;
    SharedPtr<MissionMapWindow> missionMap_;
    SharedPtr<SkillBarWindow> skillBar_;
    SharedPtr<ActorHealthBar> healthBar_;
    SharedPtr<ActorEnergyBar> energyBar_;
    SharedPtr<EffectsWindow> effectsWindow_;
    SharedPtr<DamageWindow> damageWindow_;
    SharedPtr<InventoryWindow> inventoryWindow_;
    SharedPtr<EquipmentWindow> equipWindow_;
    SharedPtr<FriendListWindow> friendsWindow_;
    SharedPtr<GuildWindow> guildWindow_;
    SharedPtr<TradeDialog> tradeDialog_;
    AB::Entities::GameType mapType_{ AB::Entities::GameTypeUnknown };
    String mapUuid_;
    String mapName_;
    uint8_t partySize_{ 1 };
    /// All objects in the scene
    HashMap<uint32_t, SharedPtr<GameObject>> objects_;
    /// Urho3D NodeIDs -> AB Object IDs given from the server
    HashMap<uint32_t, uint32_t> nodeIds_;
    WeakPtr<Actor> hoveredObject_;
    void ShowMap();
    void HideMap();
    void ToggleMap();
    void CreateUI() override;
    void SubscribeToEvents() override;
    void Update(StringHash eventType, VariantMap& eventData) override;
    void SetupViewport() override;
    void CreateScene() override;
    void PostUpdate(StringHash eventType, VariantMap& eventData) override;
    void SceneLoadingFinished() override;
private:
    IntVector2 mouseDownPos_;
    bool rmbDown_ { false };
    template<typename T>
    T* GetObjectAt(const IntVector2& pos, bool withNameLabel = false)
    {
        if (!viewport_)
            return nullptr;

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
                    if (auto* obj = GetObjectFromNode(nd))
                    {
                        if (Is<T>(obj))
                            return To<T>(obj);
                    }
                }
            }
        }

        if (!withNameLabel)
            return nullptr;

        UI* ui = GetSubsystem<UI>();
        auto* uiElem = ui->GetElementAt(pos, false);
        while (uiElem && !uiElem->IsInstanceOf<Window>() && uiElem->GetParent() != nullptr)
        {
            uiElem = uiElem->GetParent();
        }
        if (!uiElem)
            return nullptr;
        auto* actor = (GameObject*)uiElem->GetVar("Actor").GetVoidPtr();
        if (!Is<T>(actor))
            return nullptr;

        return To<T>(actor);
    }
    bool TerrainRaycast(const IntVector2& pos, Vector3& hitPos);
    void RemoveUIWindows();
    void DefaultAction();

    void HandleLevelReady(StringHash eventType, VariantMap& eventData);
    void HandleServerJoinedLeft(StringHash eventType, VariantMap& eventData);
    void HandleMouseDown(StringHash eventType, VariantMap& eventData);
    void HandleMouseUp(StringHash eventType, VariantMap& eventData);
    void HandleMouseWheel(StringHash eventType, VariantMap& eventData);
    void HandleMouseMove(StringHash eventType, VariantMap& eventData);
    void HandleObjectSpawn(StringHash eventType, VariantMap& eventData);
    void HandleObjectDespawn(StringHash eventType, VariantMap& eventData);
    void HandleObjectPosUpdate(StringHash eventType, VariantMap& eventData);
    void HandleObjectRotUpdate(StringHash eventType, VariantMap& eventData);
    void HandleObjectSetPosition(StringHash eventType, VariantMap& eventData);
    void HandleObjectStateUpdate(StringHash eventType, VariantMap& eventData);
    void HandleObjectSpeedUpdate(StringHash eventType, VariantMap& eventData);
    void HandleObjectSelected(StringHash eventType, VariantMap& eventData);
    void HandleObjectSkillFailure(StringHash eventType, VariantMap& eventData);
    void HandleObjectAttackFailure(StringHash eventType, VariantMap& eventData);
    void HandlePlayerError(StringHash eventType, VariantMap& eventData);
    void HandlePlayerAutorun(StringHash eventType, VariantMap& eventData);
    void HandleObjectEffectAdded(StringHash eventType, VariantMap& eventData);
    void HandleObjectEffectRemoved(StringHash eventType, VariantMap& eventData);
    void HandleObjectResourceChange(StringHash eventType, VariantMap& eventData);
    void HandleLogout(StringHash eventType, VariantMap& eventData);
    void HandleSelectChar(StringHash eventType, VariantMap& eventData);
    void HandleTogglePartyWindow(StringHash eventType, VariantMap& eventData);
    void HandleToggleInventoryWindow(StringHash eventType, VariantMap& eventData);
    void HandleToggleSkillsWindow(StringHash eventType, VariantMap& eventData);
    void HandleToggleEquipWindow(StringHash eventType, VariantMap& eventData);
    void HandleToggleMissionMapWindow(StringHash eventType, VariantMap& eventData);
    void HandleTargetWindowUnselectObject(StringHash eventType, VariantMap& eventData);
    void HandleToggleMap(StringHash eventType, VariantMap& eventData);
    void HandleHideUI(StringHash eventType, VariantMap& eventData);
    void HandleDefaultAction(StringHash eventType, VariantMap& eventData);
    void HandleKeepRunning(StringHash eventType, VariantMap& eventData);
    void HandleToggleChatWindow(StringHash eventType, VariantMap& eventData);
    void HandleToggleMail(StringHash eventType, VariantMap& eventData);
    void HandleReplyMail(StringHash eventType, VariantMap& eventData);
    void HandleSendMailTo(StringHash eventType, VariantMap& eventData);
    void HandleToggleNewMail(StringHash eventType, VariantMap& eventData);
    void HandleToggleFriendList(StringHash eventType, VariantMap& eventData);
    void HandleToggleGuildWindow(StringHash eventType, VariantMap& eventData);
    void HandleShowCredits(StringHash eventType, VariantMap& eventData);
    void HandleUseSkill(StringHash eventType, VariantMap& eventData);
    void HandleCancel(StringHash eventType, VariantMap& eventData);
    void HandleItemDropped(StringHash eventType, VariantMap& eventData);
    void HandleDialogTrigger(StringHash eventType, VariantMap& eventData);
    void HandleTradeDialogTrigger(StringHash eventType, VariantMap& eventData);
    void HandleTradeCancel(StringHash eventType, VariantMap& eventData);

    void SpawnObject(SpawnObjectStuct& spawnData);
};

