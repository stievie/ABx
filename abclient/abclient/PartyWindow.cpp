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
#include "PartyWindow.h"
#include "WorldLevel.h"
#include "LevelManager.h"
#include "FwClient.h"
#include "PartyItem.h"
#include "Shortcuts.h"
#include "Player.h"
#include "WindowManager.h"
#include "GameMessagesWindow.h"
#include <uuid.h>

void PartyWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<PartyWindow>();
}

PartyWindow::PartyWindow(Context* context) :
    Window(context),
    player_(nullptr)
{
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* partyFile = cache->GetResource<XMLFile>("UI/PartyWindow.xml");
    LoadXML(partyFile->GetRoot());

    // It seems this isn't loaded from the XML file
    SetLayoutMode(LM_VERTICAL);
    SetLayoutBorder(IntRect(4, 4, 4, 4));
    SetName("PartyWindow");
    SetPivot(0, 0);
    SetOpacity(0.9f);
    SetResizable(true);
    SetMovable(true);
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    SetTexture(tex);
    SetImageRect(IntRect(48, 0, 64, 16));
    SetBorder(IntRect(4, 4, 4, 4));
    SetImageBorder(IntRect(0, 0, 0, 0));
    SetResizeBorder(IntRect(8, 8, 8, 8));
    SetBringToFront(true);
    SetBringToBack(true);

    UpdateCaption();

    memberContainer_ = GetChild("MemberContainer", true);
    partyContainer_ = GetChild("PartyContainer", true);
    inviteContainer_ = GetChild("InviteContainer", true);
    addContainer_ = GetChild("AddContainer", true);
    addPlayerEdit_ = GetChildStaticCast<LineEdit>("AddPlayerEdit", true);
    invitationContainer_ = GetChild("InvitationsContainer", true);
    SetPartySize(1);

    SetSize(272, 156);
    auto* graphics = GetSubsystem<Graphics>();
    SetPosition(graphics->GetWidth() - GetWidth() - 5, graphics->GetHeight() / 2 + GetHeight());
    SetVisible(true);

    SetStyleAuto();

    SetMode(PartyWindowMode::ModeOutpost);

    SubscribeEvents();
}

PartyWindow::~PartyWindow()
{
    UnsubscribeFromAllEvents();
}

void PartyWindow::SetPlayer(SharedPtr<Player> player)
{
    player_ = player;
    if (leaderId_ == 0)
        leaderId_ = player->gameId_;
}

void PartyWindow::SetPartySize(uint8_t value)
{
    if (partySize_ != value)
    {
        partySize_ = value;
        while (memberContainers_.Size() > partySize_)
        {
            memberContainers_.Erase(memberContainers_.End());
        }
        if (memberContainers_.Size() < partySize_)
            memberContainers_.Resize(partySize_);
        for (unsigned i = 0; i < partySize_; i++)
        {
            if (!memberContainers_[i])
            {
                memberContainers_[i] = memberContainer_->CreateChild<UIElement>();
                memberContainers_[i]->SetLayoutMode(LM_VERTICAL);
            }
        }
        UpdateCaption();
    }
}

void PartyWindow::UpdateEnterButton()
{
    auto* enterContainer = GetChild("EnterContainer", true);
    auto* enterButton = GetChildStaticCast<Button>("EnterButton", true);
    if (mode_ == PartyWindowMode::ModeOutpost)
    {
        auto* lvl = GetSubsystem<LevelManager>();
        auto* game = lvl->GetGame();
        if (game)
        {
            if (!game->queueMapUuid.empty() && !uuids::uuid(game->queueMapUuid).nil())
            {
                SubscribeToEvent(enterButton, E_RELEASED, URHO3D_HANDLER(PartyWindow, HandleEnterButtonClicked));
                enterContainer->SetVisible(true);
                return;
            }
        }
    }
    UnsubscribeFromEvent(enterButton, E_RELEASED);
    enterContainer->SetVisible(false);
}

void PartyWindow::SetMode(PartyWindowMode mode)
{
    mode_ = mode;
    auto* buttonContainer = GetChild("ButtonContainer", true);
    Button* addPlayerButton = GetChildStaticCast<Button>("AddPlayerButton", true);
    if (mode == PartyWindowMode::ModeOutpost)
    {
        addContainer_->SetVisible(true);
        buttonContainer->SetVisible(true);
        SubscribeToEvent(addPlayerButton, E_RELEASED, URHO3D_HANDLER(PartyWindow, HandleAddTargetClicked));
    }
    else
    {
        UnsubscribeFromEvent(addPlayerButton, E_RELEASED);
        addContainer_->SetVisible(false);
        buttonContainer->SetVisible(false);
    }

    for (auto& cont : memberContainer_->GetChildren())
    {
        Button* kickButton = cont->GetChildDynamicCast<Button>("KickButton", true);
        if (kickButton)
        {
            if (mode_ != PartyWindowMode::ModeOutpost)
                kickButton->SetVisible(mode_ == PartyWindowMode::ModeOutpost);
        }

    }
    UpdateEnterButton();
    UpdateCaption();
}

void PartyWindow::AddItem(UIElement* container, SharedPtr<Actor> actor, MemberType type)
{
    if (!actor)
        return;

    ResourceCache* cache = GetSubsystem<ResourceCache>();

    if (type == MemberType::Member && IsInvited(actor->gameId_))
        RemoveInvite(actor->gameId_);
    UIElement* cont = container->CreateChild<UIElement>(actor->name_);
    cont->SetLayoutMode(LM_HORIZONTAL);
    cont->SetVar("ActorID", actor->gameId_);
    PartyItem* hb = cont->CreateChild<PartyItem>("HealthBar");
    hb->type_ = type;
    hb->SetAlignment(HA_LEFT, VA_TOP);
    cont->SetHeight(hb->GetHeight());
    cont->SetMaxHeight(hb->GetHeight());
    hb->SetActor(actor);
    hb->SetShowName(true);
    SubscribeToEvent(hb, E_CLICK, URHO3D_HANDLER(PartyWindow, HandleActorClicked));
    SubscribeToEvent(hb, E_DOUBLECLICK, URHO3D_HANDLER(PartyWindow, HandleActorDoubleClicked));
    switch (type)
    {
    case MemberType::Invitee:
        invitees_[actor->gameId_] = actor;
        hb->SetStyle("HealthBarGrey");
        break;
    case MemberType::Member:
        members_[actor->gameId_] = actor;
        hb->SetStyle("HealthBar");
        break;
    case MemberType::Invitation:
        invitations_[actor->gameId_] = actor;
        hb->SetStyle("HealthBarGrey");
        break;
    }

    if (mode_ == PartyWindowMode::ModeOutpost)
    {
        if (type == MemberType::Invitation)
        {
            // We got an invitation, add Accept and Reject buttons
            Button* acceptButton = cont->CreateChild<Button>();
            acceptButton->SetVar("ID", actor->gameId_);
            acceptButton->SetSize(20, 20);
            acceptButton->SetMaxSize(20, 20);
            acceptButton->SetMinSize(20, 20);
            acceptButton->SetAlignment(HA_LEFT, VA_CENTER);
            acceptButton->SetStyleAuto();
            acceptButton->SetTexture(cache->GetResource<Texture2D>("Textures/Fw-UI-Ex.png"));
            acceptButton->SetImageRect(IntRect(64, 32, 80, 48));
            acceptButton->SetHoverOffset(0, 16);
            acceptButton->SetPressedOffset(16, 0);
            SubscribeToEvent(acceptButton, E_RELEASED, URHO3D_HANDLER(PartyWindow, HandleAcceptInvitationClicked));

            Button* rejectButton = cont->CreateChild<Button>();
            rejectButton->SetVar("ID", actor->gameId_);
            rejectButton->SetSize(20, 20);
            rejectButton->SetMaxSize(20, 20);
            rejectButton->SetMinSize(20, 20);
            rejectButton->SetAlignment(HA_LEFT, VA_CENTER);
            rejectButton->SetStyleAuto();
            rejectButton->SetTexture(cache->GetResource<Texture2D>("Textures/Fw-UI-Ex.png"));
            rejectButton->SetImageRect(IntRect(96, 32, 112, 48));
            rejectButton->SetHoverOffset(0, 16);
            rejectButton->SetPressedOffset(16, 0);
            SubscribeToEvent(rejectButton, E_RELEASED, URHO3D_HANDLER(PartyWindow, HandleRejectInvitationClicked));
        }
    }
    if ((type != MemberType::Invitation) && (actor->objectType_ != ObjectType::Self) && IsLeader())
    {
        // If that's not we and we are the leader we can kick players
        Button* kickButton = cont->CreateChild<Button>("KickButton");
        kickButton->SetVar("ID", actor->gameId_);
        kickButton->SetSize(20, 20);
        kickButton->SetMaxSize(20, 20);
        kickButton->SetMinSize(20, 20);
        kickButton->SetAlignment(HA_LEFT, VA_CENTER);
        kickButton->SetStyleAuto();
        kickButton->SetTexture(cache->GetResource<Texture2D>("Textures/Fw-UI-Ex.png"));
        kickButton->SetImageRect(IntRect(96, 0, 112, 16));
        kickButton->SetHoverOffset(0, 16);
        kickButton->SetPressedOffset(16, 0);
        SubscribeToEvent(kickButton, E_RELEASED, URHO3D_HANDLER(PartyWindow, HandleKickClicked));
        kickButton->SetVisible(mode_ == PartyWindowMode::ModeOutpost);
    }

    cont->UpdateLayout();
    if (auto p = player_.Lock())
        if (p->GetSelectedObjectId() == actor->gameId_)
            SelectItem(actor->gameId_);
    UpdateAll();
}

void PartyWindow::AddMember(SharedPtr<Actor> actor, unsigned pos /* = 0 */)
{
    {
        UIElement* cont = memberContainer_->GetChild(actor->name_, true);
        if (cont)
        {
            auto pi = cont->GetChildDynamicCast<PartyItem>("HealthBar", true);
            if (pi)
            {
                pi->SetActor(actor);
                pi->SetEnabled(true);
                members_[actor->gameId_] = actor;
                // Update ID of kick button
                Button* kickButton = cont->GetChildDynamicCast<Button>("KickButton", false);
                if (kickButton)
                {
                    if (mode_ != PartyWindowMode::ModeOutpost)
                        kickButton->SetVisible(false);
                    // Only leader has that and only in outposts
                    kickButton->SetVar("ID", actor->gameId_);
                }
                return;
            }
        }
    }

    unsigned p = (pos != 0) ? pos - 1 : members_.Size();
    if (p < memberContainers_.Size())
    {
        UIElement* cont = memberContainers_[p];
        AddItem(cont, actor, MemberType::Member);
    }
}

void PartyWindow::AddInvitee(SharedPtr<Actor> actor)
{
    AddItem(inviteContainer_, actor, MemberType::Invitee);
}

void PartyWindow::AddInvitation(SharedPtr<Actor> leader)
{
    AddItem(invitationContainer_, leader, MemberType::Invitation);
}

void PartyWindow::RemoveMember(uint32_t actorId)
{
    auto item = GetItem(actorId);
    if (item && item->type_ == MemberType::Member)
    {
        item->GetParent()->Remove();
        members_.Erase(actorId);
        UpdateAll();
    }
    else
        URHO3D_LOGWARNINGF("Member with ID %d not found", actorId);
}

void PartyWindow::RemoveInvite(uint32_t actorId)
{
    auto item = GetItem(actorId);
    if (item && item->type_ == MemberType::Invitee)
    {
        item->GetParent()->Remove();
        invitees_.Erase(actorId);
        UpdateAll();
    }
    else
        URHO3D_LOGWARNINGF("Invitee with ID %d not found", actorId);
}

void PartyWindow::RemoveInvitation(uint32_t actorId)
{
    auto item = GetItem(actorId);
    if (item && item->type_ == MemberType::Invitation)
    {
        item->GetParent()->Remove();
        invitations_.Erase(actorId);
        UpdateAll();
    }
    else
        URHO3D_LOGWARNINGF("Invitation with ID %d not found", actorId);
}

void PartyWindow::RemoveActor(uint32_t actorId)
{
    auto item = GetItem(actorId);
    if (item)
    {
        switch (item->type_)
        {
        case MemberType::Invitation:
            invitations_.Erase(actorId);
            break;
        case MemberType::Invitee:
            invitees_.Erase(actorId);
            break;
        case MemberType::Member:
            members_.Erase(actorId);
            break;
        }
        item->GetParent()->Remove();
        UpdateAll();
    }
}

void PartyWindow::Clear()
{
    const auto& children = memberContainer_->GetChildren();
    for (auto& child : children)
        child->RemoveAllChildren();
    inviteContainer_->RemoveAllChildren();
    invitationContainer_->RemoveAllChildren();
    members_.Clear();
    invitees_.Clear();
    invitations_.Clear();
    UpdateAll();
}

void PartyWindow::HandleAddTargetClicked(StringHash, VariantMap&)
{
    if (mode_ != PartyWindowMode::ModeOutpost)
        return;
    if (IsFull())
    {
        ShowError("The party is full.");
        return;
    }
    uint32_t targetId = 0;
    LevelManager* lm = GetSubsystem<LevelManager>();
    auto* a = lm->GetActorByName(addPlayerEdit_->GetText());
    if (a)
    {
        if (auto p = player_.Lock())
        {
            if (a->gameId_ == p->gameId_)
            {
                ShowError("You can not invite yourself!");
                return;
            }
        }
        targetId = a->gameId_;
    }
    if (targetId != 0)
    {
        FwClient* client = GetSubsystem<FwClient>();
        client->PartyInvitePlayer(targetId);
    }
    else
        ShowError("This player is not online.");
}

void PartyWindow::HandleEnterButtonClicked(StringHash, VariantMap&)
{
    auto* client = GetSubsystem<FwClient>();
    client->QueueMatch();
}

void PartyWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}

void PartyWindow::HandleLeaveButtonClicked(StringHash, VariantMap&)
{
    if (mode_ != PartyWindowMode::ModeOutpost)
        return;
    FwClient* cli = GetSubsystem<FwClient>();
    cli->PartyLeave();
}

void PartyWindow::HandleObjectSelected(StringHash, VariantMap& eventData)
{
    // An object was selected in the world
    using namespace Events::ObjectSelected;
    uint32_t sourceId = eventData[P_SOURCEID].GetUInt();
    if (auto p = player_.Lock())
    {
        // This object was not selected by us
        if (sourceId != p->gameId_)
            return;
    }
    else
        return;

    uint32_t targetId = eventData[P_TARGETID].GetUInt();
    if (SelectItem(targetId))
        return;

    if (targetId == 0)
        return;

    if (!addPlayerEdit_)
        return;
    if (mode_ == PartyWindowMode::ModeOutpost)
    {
        LevelManager* lm = GetSubsystem<LevelManager>();
        auto* o = lm->GetObject(targetId);
        if (o && o->objectType_ == ObjectType::Player)
        {
            Actor& a = To<Actor>(*o);
            addPlayerEdit_->SetText(a.name_);
        }
    }
}

void PartyWindow::HandlePartyInvited(StringHash, VariantMap& eventData)
{
    // The leader invited someone
    if (mode_ != PartyWindowMode::ModeOutpost)
        return;
    using namespace Events::PartyInvited;
    uint32_t sourceId = eventData[P_SOURCEID].GetUInt();
    uint32_t targetId = eventData[P_TARGETID].GetUInt();
    LevelManager* lm = GetSubsystem<LevelManager>();
    GameObject* o = lm->GetObject(targetId);
    if (o)
    {
        if (Is<Player>(o))
        {
            // We got an invitation from another group
            GameObject* leader = lm->GetObject(sourceId);
            if (leader)
                AddInvitation(SharedPtr<Actor>(To<Actor>(leader)));
        }
        else if (o->objectType_ == ObjectType::Player)
        {
            // We invited another player
            AddInvitee(SharedPtr<Actor>(To<Actor>(o)));
        }
    }
}

void PartyWindow::HandlePartyAdded(StringHash, VariantMap& eventData)
{
/*
URHO3D_PARAM(P_UPDATETICK, UpdateTick);
URHO3D_PARAM(P_PLAYERID, PlayerId);     // unit32_t
URHO3D_PARAM(P_LEADERID, LeaderId);     // unit32_t
URHO3D_PARAM(P_PARTYID, PartyId);       // unit32_t
*/
    // A player accepted
    using namespace Events::PartyAdded;
    uint32_t actorId = eventData[P_PLAYERID].GetUInt();
    uint32_t leaderId = eventData[P_LEADERID].GetUInt();
    if (leaderId != 0)
        leaderId_ = leaderId;
    uint32_t partyId = eventData[P_PARTYID].GetUInt();
    LevelManager* lm = GetSubsystem<LevelManager>();
    GameObject* o = lm->GetObject(actorId);
    if (Is<Player>(o))
    {
        if (auto p = player_.Lock())
            p->groupId_ = partyId;
        groupId_ = partyId;
        // We was added to a new party
        Clear();
        To<Player>(*o).UpdateMumbleContext();
        // Get full list of members
        FwClient* cli = GetSubsystem<FwClient>();
        cli->PartyGetMembers(partyId);
    }
    else if (Is<Actor>(o) && o->objectType_ == ObjectType::Player)
    {
        Actor* actor = To<Actor>(o);
        actor->groupId_ = partyId;
        AddMember(SharedPtr<Actor>(actor));
    }
}

void PartyWindow::HandlePartyInviteRemoved(StringHash, VariantMap& eventData)
{
    using namespace Events::PartyInviteRemoved;
/*
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_SOURCEID, SourceId);     // unit32_t
    URHO3D_PARAM(P_TARGETID, TargetId);     // unit32_t
    URHO3D_PARAM(P_PARTYID, PartyId);       // unit32_t
*/
    uint32_t targetId = eventData[P_TARGETID].GetUInt();
    if (auto p = player_.Lock())
    {
        if (p->gameId_ == targetId)
        {
            // Removed our invitation
            uint32_t sourceId = eventData[P_SOURCEID].GetUInt();
            RemoveInvitation(sourceId);
        }
        else
        {
            // Removed someone else's  invitation
            RemoveInvite(targetId);
        }
        UpdateAll();
    }
}

void PartyWindow::HandlePartyRemoved(StringHash, VariantMap& eventData)
{
    // Party member was removed
    using namespace Events::PartyRemoved;
    uint32_t targetId = eventData[P_TARGETID].GetUInt();
    LevelManager* lm = GetSubsystem<LevelManager>();
    GameObject* o = lm->GetObject(targetId);
    if (Is<Player>(o))
    {
        // We get a new party
        groupId_ = 0;
        To<Player>(*o).UpdateMumbleContext();
        ClearMembers();
        return;
    }
    RemoveMember(targetId);
}

void PartyWindow::HandleActorClicked(StringHash, VariantMap& eventData)
{
    // An actor was clicked in the Party window
    using namespace Click;
    PartyItem* hb = static_cast<PartyItem*>(eventData[P_ELEMENT].GetPtr());
    SharedPtr<Actor> actor = hb->GetActor();
    if (actor)
    {
        if (auto p = player_.Lock())
        {
            p->SelectObject(actor->gameId_);
        }
    }
}

void PartyWindow::HandleActorDoubleClicked(StringHash, VariantMap& eventData)
{
    // An actor was double clicked in the Party window
    using namespace DoubleClick;
    PartyItem* hb = static_cast<PartyItem*>(eventData[P_ELEMENT].GetPtr());
    SharedPtr<Actor> actor = hb->GetActor();
    if (actor)
    {
        // Should be selected in Click
        VariantMap& e = GetEventDataMap();
        SendEvent(Events::E_SC_DEFAULTACTION, e);
    }
}

void PartyWindow::HandleAcceptInvitationClicked(StringHash, VariantMap& eventData)
{
    if (mode_ != PartyWindowMode::ModeOutpost)
        return;
    using namespace Released;
    auto* elem = static_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
    uint32_t actorId = elem->GetVar("ID").GetUInt();
    auto item = GetItem(actorId);
    if (!item)
        return;

    FwClient* cli = GetSubsystem<FwClient>();
    cli->PartyAcceptInvite(actorId);
}

void PartyWindow::HandleRejectInvitationClicked(StringHash, VariantMap& eventData)
{
    if (mode_ != PartyWindowMode::ModeOutpost)
        return;
    using namespace Released;
    auto* elem = static_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
    uint32_t actorId = elem->GetVar("ID").GetUInt();

    FwClient* cli = GetSubsystem<FwClient>();
    cli->PartyRejectInvite(actorId);
}

void PartyWindow::HandleKickClicked(StringHash, VariantMap& eventData)
{
    if (mode_ != PartyWindowMode::ModeOutpost)
        return;
    using namespace Released;
    auto* elem = static_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
    uint32_t actorId = elem->GetVar("ID").GetUInt();

    FwClient* cli = GetSubsystem<FwClient>();
    cli->PartyKickPlayer(actorId);
}

void PartyWindow::HandleObjectDespawn(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectDespawn;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    RemoveActor(objectId);
}

void PartyWindow::HandlePartyInfoMembers(StringHash, VariantMap& eventData)
{
    using namespace Events::PartyInfoMembers;
    uint32_t partyId = eventData[P_PARTYID].GetUInt();
    VariantVector members = eventData[P_MEMBERS].GetVariantVector();
    if (members_.Size() != 0)
        return;
    if (auto p = player_.Lock())
    {
        if (p->groupId_ != partyId)
            return;
    }
    else
        return;

    ClearMembers();
    leaderId_ = 0;
    for (auto m : members)
    {
        if (leaderId_ == 0)
            leaderId_ = m.GetUInt();                       // First is leader
        LevelManager* lm = GetSubsystem<LevelManager>();
        GameObject* o = lm->GetObject(m.GetUInt());
        if (Is<Actor>(o))
        {
            Actor* actor = To<Actor>(o);
            actor->groupId_ = eventData[P_PARTYID].GetUInt();
            AddMember(SharedPtr<Actor>(actor));
        }
    }
}

void PartyWindow::HandleLeaveInstance(StringHash, VariantMap&)
{
    // These are added with their new ID when they spawn
    members_.Clear();
    ClearInvitations();
    ClearInvites();
}

void PartyWindow::HandleTargetPinged(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectPingTarget;
    uint32_t targetId = eventData[P_TARGETID].GetUInt();
    LevelManager* lm = GetSubsystem<LevelManager>();
    target_ = lm->GetObject(targetId);
}

void PartyWindow::HandleSelectTarget(StringHash, VariantMap&)
{
    if (auto t = target_.Lock())
    {
        player_->SelectObject(t->gameId_);
    }
}

void PartyWindow::SubscribeEvents()
{
    Button* closeButton = GetChildStaticCast<Button>("CloseButton", true);
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(PartyWindow, HandleCloseClicked));
    Button* leaveButton = GetChildStaticCast<Button>("LeaveButton", true);
    SubscribeToEvent(leaveButton, E_RELEASED, URHO3D_HANDLER(PartyWindow, HandleLeaveButtonClicked));

    SubscribeToEvent(Events::E_LEAVEINSTANCE, URHO3D_HANDLER(PartyWindow, HandleLeaveInstance));
    SubscribeToEvent(Events::E_OBJECTDESPAWN, URHO3D_HANDLER(PartyWindow, HandleObjectDespawn));
    SubscribeToEvent(Events::E_OBJECTSELECTED, URHO3D_HANDLER(PartyWindow, HandleObjectSelected));
    SubscribeToEvent(Events::E_PARTYADDED, URHO3D_HANDLER(PartyWindow, HandlePartyAdded));
    SubscribeToEvent(Events::E_PARTYINVITED, URHO3D_HANDLER(PartyWindow, HandlePartyInvited));
    SubscribeToEvent(Events::E_PARTYINVITEREMOVED, URHO3D_HANDLER(PartyWindow, HandlePartyInviteRemoved));
    SubscribeToEvent(Events::E_PARTYREMOVED, URHO3D_HANDLER(PartyWindow, HandlePartyRemoved));
    SubscribeToEvent(Events::E_PARTYINFOMEMBERS, URHO3D_HANDLER(PartyWindow, HandlePartyInfoMembers));
    SubscribeToEvent(Events::E_OBJECTPINGTARGET, URHO3D_HANDLER(PartyWindow, HandleTargetPinged));
    SubscribeToEvent(Events::E_SC_SELECTTARGET, URHO3D_HANDLER(PartyWindow, HandleSelectTarget));
}

void PartyWindow::UpdateCaption()
{
    Shortcuts* scs = GetSubsystem<Shortcuts>();
    Text* caption = GetChildStaticCast<Text>("Caption", true);
    String s(scs->GetCaption(Events::E_SC_TOGGLEPARTYWINDOW, "Party", true));
    if (mode_ == PartyWindowMode::ModeOutpost)
        s += "  " + String(members_.Size()) + "/" + String(static_cast<int>(partySize_));
    caption->SetText(s);
}

void PartyWindow::UpdateAll()
{
    partyContainer_->SetMinHeight(memberContainer_->GetHeight() + inviteContainer_->GetHeight() + 25);
    SetMinHeight(partyContainer_->GetHeight() + invitationContainer_->GetHeight() + 33 + 30);
    UpdateCaption();
    UpdateLayout();
}

void PartyWindow::ClearMembers()
{
    const auto& children = memberContainer_->GetChildren();
    for (auto& child : children)
        child->RemoveAllChildren();
    members_.Clear();
}

void PartyWindow::ClearInvitations()
{
    invitationContainer_->RemoveAllChildren();
    invitations_.Clear();
}

void PartyWindow::ClearInvites()
{
    inviteContainer_->RemoveAllChildren();
    invitees_.Clear();
}

void PartyWindow::ShowError(const String& msg)
{
    WindowManager* wm = GetSubsystem<WindowManager>();
    GameMessagesWindow* wnd = dynamic_cast<GameMessagesWindow*>(wm->GetWindow(WINDOW_GAMEMESSAGES, true).Get());
    wnd->ShowError(msg);
}

PartyItem* PartyWindow::GetItem(uint32_t actorId)
{
    if (members_.Contains(actorId))
    {
        auto actor = members_[actorId].Lock();
        if (actor)
        {
            UIElement* cont = memberContainer_->GetChild(actor->name_, true);
            if (cont)
            {
                auto pi = cont->GetChild("HealthBar", true);
                if (pi)
                    return dynamic_cast<PartyItem*>(pi);
            }
        }
    }
    if (invitees_.Contains(actorId))
    {
        auto actor = invitees_[actorId].Lock();
        if (actor)
        {
            UIElement* cont = inviteContainer_->GetChild(actor->name_);
            if (cont)
            {
                auto pi = cont->GetChild("HealthBar", true);
                if (pi)
                    return dynamic_cast<PartyItem*>(pi);
            }
        }
    }
    if (invitations_.Contains(actorId))
    {
        auto actor = invitations_[actorId].Lock();
        if (actor)
        {
            UIElement* cont = invitationContainer_->GetChild(actor->name_);
            if (cont)
            {
                auto pi = cont->GetChild("HealthBar", true);
                if (pi)
                    return dynamic_cast<PartyItem*>(pi);
            }
        }
    }
    return nullptr;
}

void PartyWindow::UnselectAll()
{
    auto members = memberContainer_->GetChildren();
    for (auto& cont : members)
    {
        auto pi = cont->GetChildDynamicCast<PartyItem>("HealthBar", true);
        if (pi)
            pi->SetSelected(false);
    }
    auto invites = inviteContainer_->GetChildren();
    for (auto& cont : invites)
    {
        auto pi = cont->GetChildDynamicCast<PartyItem>("HealthBar", false);
        if (pi)
            pi->SetSelected(false);
    }
    auto inv = invitationContainer_->GetChildren();
    for (auto& cont : inv)
    {
        auto pi = cont->GetChildDynamicCast<PartyItem>("HealthBar", false);
        if (pi)
            pi->SetSelected(false);
    }
}

bool PartyWindow::SelectItem(uint32_t actorId)
{
    UnselectAll();
    auto item = GetItem(actorId);
    if (item)
    {
        item->SetSelected(true);
        return true;
    }
    return false;
}

bool PartyWindow::UnselectItem(uint32_t actorId)
{
    auto item = GetItem(actorId);
    if (item)
    {
        item->SetSelected(false);
        return true;
    }
    return false;
}

void PartyWindow::OnObjectSpawned(GameObject* object, uint32_t groupId, uint8_t groupPos)
{
    if (Is<Actor>(object))
    {
        if (object->objectType_ == ObjectType::Self)
            groupId_ = groupId;

//        URHO3D_LOGINFOF("Object spawned: objectId = %d, groupId = %d, pos = %d, My groupid = %d", object->gameId_, groupId, groupPos, groupId_);
        if (groupId == groupId_)
            AddMember(SharedPtr<Actor>(To<Actor>(object)), groupPos);
    }
}

bool PartyWindow::IsLeader()
{
    if (!player_)
        return false;

    UIElement* elem = memberContainers_[0];
    auto pi = elem->GetChild("HealthBar", true);
    if (pi)
    {
        PartyItem* item = dynamic_cast<PartyItem*>(pi);
        if (item)
            return item->GetActor()->gameId_ == player_->gameId_;
    }
    return false;
}
