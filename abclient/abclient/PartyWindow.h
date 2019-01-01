#pragma once

#include "PartyItem.h"

class Actor;
class Player;
class PartyItem;

enum class PartyWindowMode
{
    ModeOutpost,
    ModeGame
};

class PartyWindow : public Window
{
    URHO3D_OBJECT(PartyWindow, Window);
private:
    PartyWindowMode mode_;
    SharedPtr<Window> window_;
    SharedPtr<LineEdit> addPlayerEdit_;
    uint8_t partySize_;
    SharedPtr<UIElement> memberContainer_;
    SharedPtr<UIElement> partyContainer_;
    SharedPtr<UIElement> addContainer_;
    SharedPtr<UIElement> inviteContainer_;
    SharedPtr<UIElement> invitationContainer_;
    Vector<SharedPtr<UIElement>> memberContainers_;
    WeakPtr<Player> player_;
    HashMap<uint32_t, WeakPtr<Actor>> members_;
    HashMap<uint32_t, WeakPtr<Actor>> invitees_;
    HashMap<uint32_t, WeakPtr<Actor>> invitations_;
    uint32_t leaderId_;
    uint32_t groupId_;
    void HandleAddTargetClicked(StringHash eventType, VariantMap& eventData);
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleLeaveButtonClicked(StringHash eventType, VariantMap& eventData);
    void HandleObjectSelected(StringHash eventType, VariantMap& eventData);
    void HandlePartyInvited(StringHash eventType, VariantMap& eventData);
    void HandlePartyAdded(StringHash eventType, VariantMap& eventData);
    void HandlePartyInviteRemoved(StringHash eventType, VariantMap& eventData);
    void HandlePartyRemoved(StringHash eventType, VariantMap& eventData);
    void HandleActorClicked(StringHash eventType, VariantMap& eventData);
    void HandleAcceptInvitationClicked(StringHash eventType, VariantMap& eventData);
    void HandleRejectInvitationClicked(StringHash eventType, VariantMap& eventData);
    void HandleKickClicked(StringHash eventType, VariantMap& eventData);
    void HandleObjectDespawn(StringHash eventType, VariantMap& eventData);
    void HandlePartyInfoMembers(StringHash eventType, VariantMap& eventData);
    void HandleLeaveInstance(StringHash eventType, VariantMap& eventData);
    void SubscribeEvents();
    void UpdateCaption();
    void UpdateAll();
    void ClearMembers();
    void ClearInvitations();
    void ShowError(const String& msg);
    bool IsFull() const { return members_.Size() >= partySize_; }
    PartyItem* GetItem(uint32_t actorId);
    void AddItem(UIElement* container, SharedPtr<Actor> actor, MemberType type);
public:
    static void RegisterObject(Context* context);

    PartyWindow(Context* context);
    ~PartyWindow();

    void SetPlayer(SharedPtr<Player> player);
    void SetPartySize(uint8_t value);
    void SetMode(PartyWindowMode mode);
    void AddMember(SharedPtr<Actor> actor, unsigned pos = 0);
    void RemoveMember(uint32_t actorId);
    void AddInvitee(SharedPtr<Actor> actor);
    void AddInvitation(SharedPtr<Actor> leader);
    void RemoveInvite(uint32_t actorId);
    void RemoveInvitation(uint32_t actorId);
    void RemoveActor(uint32_t actorId);
    void Clear();
    void UnselectAll();
    bool SelectItem(uint32_t actorId);
    bool UnselectItem(uint32_t actorId);
    void OnObjectSpawned(GameObject* object, uint32_t groupId, uint8_t groupPos);
    bool IsLeader();
};

