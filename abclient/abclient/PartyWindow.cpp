#include "stdafx.h"
#include "PartyWindow.h"
#include "AbEvents.h"
#include "WorldLevel.h"
#include "LevelManager.h"
#include "FwClient.h"
#include "PartyItem.h"
#include "Shortcuts.h"
#include "Player.h"
#include "WindowManager.h"
#include "GameMessagesWindow.h"

void PartyWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<PartyWindow>();
}

PartyWindow::PartyWindow(Context* context) :
    Window(context),
    player_(nullptr),
    leaderId_(0),
    partySize_(0),
    groupId_(0)
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

    UpdateCaption();

    memberContainer_ = dynamic_cast<UIElement*>(GetChild("MemberContainer", true));
    partyContainer_ = dynamic_cast<UIElement*>(GetChild("PartyContainer", true));
    inviteContainer_ = dynamic_cast<UIElement*>(GetChild("InviteContainer", true));
    addContainer_ = dynamic_cast<UIElement*>(GetChild("AddContainer", true));
    addPlayerEdit_ = dynamic_cast<LineEdit*>(GetChild("AddPlayerEdit", true));
    invitationContainer_ = dynamic_cast<UIElement*>(GetChild("InvitationsContainer", true));
    SetPartySize(1);

    SetSize(272, 156);
    auto* graphics = GetSubsystem<Graphics>();
    SetPosition(graphics->GetWidth() - GetWidth() - 5, graphics->GetHeight() / 2 + GetHeight());
    SetVisible(true);

    SetStyleAuto();

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
        leaderId_ = player->id_;
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

void PartyWindow::SetMode(PartyWindowMode mode)
{
    mode_ = mode;
    auto* buttonContainer = dynamic_cast<UIElement*>(GetChild("ButtonContainer", true));
    Button* addPlayerButton = dynamic_cast<Button*>(GetChild("AddPlayerButton", true));
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
    UpdateCaption();
}

void PartyWindow::AddItem(UIElement* container, SharedPtr<Actor> actor, MemberType type)
{
    if (!actor)
        return;

    ResourceCache* cache = GetSubsystem<ResourceCache>();

    if (type == MemberType::Member)
        RemoveInvite(actor->id_);
    UIElement* cont = container->CreateChild<UIElement>(actor->name_);
    cont->SetLayoutMode(LM_HORIZONTAL);
    cont->SetVar("ActorID", actor->id_);
    PartyItem* hb = cont->CreateChild<PartyItem>("HealthBar");
    hb->type_ = type;
    hb->SetAlignment(HA_LEFT, VA_TOP);
    cont->SetHeight(hb->GetHeight());
    cont->SetMaxHeight(hb->GetHeight());
    hb->SetActor(actor);
    hb->SetShowName(true);
    SubscribeToEvent(hb, E_CLICK, URHO3D_HANDLER(PartyWindow, HandleActorClicked));
    switch (type)
    {
    case MemberType::Invitee:
        invitees_[actor->id_] = actor;
        hb->SetStyle("HealthBarGrey");
        break;
    case MemberType::Member:
        members_[actor->id_] = actor;
        hb->SetStyle("HealthBar");
        break;
    case MemberType::Invitation:
        invitations_[actor->id_] = actor;
        hb->SetStyle("HealthBarGrey");
        break;
    }

    if (mode_ == PartyWindowMode::ModeOutpost)
    {
        if (type == MemberType::Invitation)
        {
            // We got an invitation, add Accept and Reject buttons
            Button* acceptButton = cont->CreateChild<Button>();
            acceptButton->SetVar("ID", actor->id_);
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
            rejectButton->SetVar("ID", actor->id_);
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
        else if (actor->objectType_ != ObjectTypeSelf && IsLeader())
        {
            // If thats not we and we are the leader we can kick players
            Button* kickButton = cont->CreateChild<Button>();
            kickButton->SetVar("ID", actor->id_);
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
        }
    }
    cont->UpdateLayout();
    if (auto p = player_.Lock())
        if (p->GetSelectedObjectId() == actor->id_)
            SelectItem(actor->id_);
    UpdateAll();
}

void PartyWindow::AddMember(SharedPtr<Actor> actor, unsigned pos /* = 0 */)
{
    {
        UIElement* cont = memberContainer_->GetChild(actor->name_, true);
        if (cont)
        {
            auto pi = dynamic_cast<PartyItem*>(cont->GetChild("HealthBar", true));
            if (pi)
            {
                pi->SetActor(actor);
                pi->SetEnabled(true);
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
    if (item)
    {
        item->GetParent()->Remove();
        members_.Erase(actorId);
        UpdateAll();
    }
}

void PartyWindow::RemoveInvite(uint32_t actorId)
{
    auto item = GetItem(actorId);
    if (item)
    {
        item->GetParent()->Remove();
        invitees_.Erase(actorId);
        UpdateAll();
    }
}

void PartyWindow::RemoveInvitation(uint32_t actorId)
{
    auto item = GetItem(actorId);
    if (item)
    {
        item->GetParent()->Remove();
        invitations_.Erase(actorId);
        UpdateAll();
    }
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
    SharedPtr<Actor> a = lm->GetActorByName(addPlayerEdit_->GetText());
    if (a)
        targetId = a->id_;
    if (targetId != 0)
    {
        FwClient* client = GetSubsystem<FwClient>();
        client->PartyInvitePlayer(targetId);
    }
    else
        ShowError("This player is not online.");
}

void PartyWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}

void PartyWindow::HandleLeaveButtonClicked(StringHash, VariantMap&)
{
    FwClient* cli = GetSubsystem<FwClient>();
    cli->PartyLeave();
}

void PartyWindow::HandleObjectSelected(StringHash, VariantMap& eventData)
{
    // An object was selected in the world
    using namespace AbEvents::ObjectSelected;
    uint32_t sourceId = eventData[P_SOURCEID].GetUInt();
    if (auto p = player_.Lock())
    {
        // This object was not selected by us
        if (sourceId != p->id_)
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
        SharedPtr<GameObject> o = lm->GetObjectById(targetId);
        if (o && o->objectType_ == ObjectTypePlayer)
        {
            Actor* a = dynamic_cast<Actor*>(o.Get());
            if (a)
            {
                addPlayerEdit_->SetText(a->name_);
            }
        }
    }
}

void PartyWindow::HandlePartyInvited(StringHash, VariantMap& eventData)
{
    // The leader invited someone
    if (mode_ != PartyWindowMode::ModeOutpost)
        return;
    using namespace AbEvents::PartyInvited;
    uint32_t sourceId = eventData[P_SOURCEID].GetUInt();
    uint32_t targetId = eventData[P_TARGETID].GetUInt();
//    uint32_t partyId = eventData[P_PARTYID].GetUInt();
    LevelManager* lm = GetSubsystem<LevelManager>();
    GameObject* o = lm->GetObjectById(targetId);
    if (o)
    {
        if (o->objectType_ == ObjectTypePlayer)
        {
            // We invited another player
            AddInvitee(SharedPtr<Actor>(dynamic_cast<Actor*>(o)));
        }
        else if (o->objectType_ == ObjectTypeSelf)
        {
            // We got an invitation from another group
            GameObject* leader = lm->GetObjectById(sourceId);
            if (leader)
                AddInvitation(SharedPtr<Actor>(dynamic_cast<Actor*>(leader)));
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
    using namespace AbEvents::PartyAdded;
    uint32_t actorId = eventData[P_PLAYERID].GetUInt();
    uint32_t leaderId = eventData[P_LEADERID].GetUInt();
    if (leaderId != 0)
        leaderId_ = leaderId;
    uint32_t partyId = eventData[P_PARTYID].GetUInt();
    LevelManager* lm = GetSubsystem<LevelManager>();
    GameObject* o = lm->GetObjectById(actorId);
    if (o)
    {
        if (o->objectType_ == ObjectTypePlayer)
        {
            Actor* actor = dynamic_cast<Actor*>(o);
            actor->groupId_ = partyId;
            AddMember(SharedPtr<Actor>(dynamic_cast<Actor*>(o)));
        }
        else if (o->objectType_ == ObjectTypeSelf)
        {
            if (auto p = player_.Lock())
                p->groupId_ = partyId;
            groupId_ = partyId;
            // We was added to a new party
            Clear();
            // Get full list of members
            FwClient* cli = GetSubsystem<FwClient>();
            cli->PartyGetMembers(partyId);
        }
    }
}

void PartyWindow::HandlePartyInviteRemoved(StringHash, VariantMap& eventData)
{
    using namespace AbEvents::PartyInviteRemoved;
/*
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_SOURCEID, SourceId);     // unit32_t
    URHO3D_PARAM(P_TARGETID, TargetId);     // unit32_t
    URHO3D_PARAM(P_PARTYID, PartyId);       // unit32_t
*/
    uint32_t targetId = eventData[P_TARGETID].GetUInt();
    if (auto p = player_.Lock())
    {
        if (p->id_ == targetId)
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
    using namespace AbEvents::PartyRemoved;
    uint32_t targetId = eventData[P_TARGETID].GetUInt();
    LevelManager* lm = GetSubsystem<LevelManager>();
    GameObject* o = lm->GetObjectById(targetId);
    if (o)
    {
        if (o->objectType_ == ObjectTypeSelf)
        {
            // We get a new party
            groupId_ = 0;
            ClearMembers();
            return;
        }
    }
    RemoveMember(targetId);
}

void PartyWindow::HandleActorClicked(StringHash, VariantMap& eventData)
{
    // An actor was clicked in the Party window
    using namespace Click;
    PartyItem* hb = dynamic_cast<PartyItem*>(eventData[P_ELEMENT].GetPtr());
    SharedPtr<Actor> actor = hb->GetActor();
    if (actor)
    {
        if (auto p = player_.Lock())
        {
            p->SelectObject(actor->id_);
        }
    }
}

void PartyWindow::HandleAcceptInvitationClicked(StringHash, VariantMap& eventData)
{
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
    using namespace Released;
    auto* elem = static_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
    uint32_t actorId = elem->GetVar("ID").GetUInt();
    auto item = GetItem(actorId);
    if (!item)
        return;

    FwClient* cli = GetSubsystem<FwClient>();
    cli->PartyRejectInvite(actorId);
}

void PartyWindow::HandleKickClicked(StringHash, VariantMap& eventData)
{
    using namespace Released;
    auto* elem = static_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
    uint32_t actorId = elem->GetVar("ID").GetUInt();
    auto item = GetItem(actorId);
    if (!item)
        return;

    FwClient* cli = GetSubsystem<FwClient>();
    cli->PartyKickPlayer(actorId);
}

void PartyWindow::HandleObjectDespawn(StringHash, VariantMap& eventData)
{
    using namespace AbEvents::ObjectDespawn;
    uint32_t objectId = eventData[P_OBJECTID].GetInt();
    RemoveActor(objectId);
}

void PartyWindow::HandlePartyInfoMembers(StringHash, VariantMap& eventData)
{
    using namespace AbEvents::PartyInfoMembers;
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
        GameObject* o = lm->GetObjectById(m.GetUInt());
        if (o)
        {
            Actor* actor = dynamic_cast<Actor*>(o);
            actor->groupId_ = eventData[P_PARTYID].GetUInt();
            AddMember(SharedPtr<Actor>(dynamic_cast<Actor*>(o)));
        }
    }
}

void PartyWindow::HandleLeaveInstance(StringHash, VariantMap&)
{
    ClearInvitations();
    ClearInvites();
}

void PartyWindow::SubscribeEvents()
{
    Button* closeButton = dynamic_cast<Button*>(GetChild("CloseButton", true));
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(PartyWindow, HandleCloseClicked));
    Button* leaveButton = dynamic_cast<Button*>(GetChild("LeaveButton", true));
    SubscribeToEvent(leaveButton, E_RELEASED, URHO3D_HANDLER(PartyWindow, HandleLeaveButtonClicked));
    SubscribeToEvent(AbEvents::E_LEAVEINSTANCE, URHO3D_HANDLER(PartyWindow, HandleLeaveInstance));
    SubscribeToEvent(AbEvents::E_OBJECTDESPAWN, URHO3D_HANDLER(PartyWindow, HandleObjectDespawn));
    SubscribeToEvent(AbEvents::E_OBJECTSELECTED, URHO3D_HANDLER(PartyWindow, HandleObjectSelected));
    SubscribeToEvent(AbEvents::E_PARTYADDED, URHO3D_HANDLER(PartyWindow, HandlePartyAdded));
    SubscribeToEvent(AbEvents::E_PARTYINVITED, URHO3D_HANDLER(PartyWindow, HandlePartyInvited));
    SubscribeToEvent(AbEvents::E_PARTYINVITEREMOVED, URHO3D_HANDLER(PartyWindow, HandlePartyInviteRemoved));
    SubscribeToEvent(AbEvents::E_PARTYREMOVED, URHO3D_HANDLER(PartyWindow, HandlePartyRemoved));
    SubscribeToEvent(AbEvents::E_PARTYINFOMEMBERS, URHO3D_HANDLER(PartyWindow, HandlePartyInfoMembers));
}

void PartyWindow::UpdateCaption()
{
    Shortcuts* scs = GetSubsystem<Shortcuts>();
    Text* caption = dynamic_cast<Text*>(GetChild("Caption", true));
    String s(scs->GetCaption(AbEvents::E_SC_TOGGLEPARTYWINDOW, "Party", true));
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
        UIElement* cont = memberContainer_->GetChild(actor->name_, true);
        if (cont)
        {
            auto pi = cont->GetChild("HealthBar", true);
            if (pi)
                return dynamic_cast<PartyItem*>(pi);
        }
    }
    if (invitees_.Contains(actorId))
    {
        auto actor = invitees_[actorId].Lock();
        UIElement* cont = inviteContainer_->GetChild(actor->name_);
        if (cont)
        {
            auto pi = cont->GetChild("HealthBar", true);
            if (pi)
                return dynamic_cast<PartyItem*>(pi);
        }
    }
    if (invitations_.Contains(actorId))
    {
        auto actor = invitations_[actorId].Lock();
        UIElement* cont = invitationContainer_->GetChild(actor->name_);
        if (cont)
        {
            auto pi = cont->GetChild("HealthBar", true);
            if (pi)
                return dynamic_cast<PartyItem*>(pi);
        }
    }

    return nullptr;
}

void PartyWindow::UnselectAll()
{
    auto members = memberContainer_->GetChildren();
    for (auto& cont : members)
    {
        auto pi = dynamic_cast<PartyItem*>(cont->GetChild("HealthBar", true));
        if (pi)
            pi->SetSelected(false);
    }
    auto invites = inviteContainer_->GetChildren();
    for (auto& cont : invites)
    {
        auto pi = dynamic_cast<PartyItem*>(cont->GetChild("HealthBar", false));
        if (pi)
            pi->SetSelected(false);
    }
    auto inv = invitationContainer_->GetChildren();
    for (auto& cont : inv)
    {
        auto pi = dynamic_cast<PartyItem*>(cont->GetChild("HealthBar", false));
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
    if (object)
    {
        if (object->objectType_ == ObjectTypeSelf)
            groupId_ = groupId;

        URHO3D_LOGINFOF("Object spawned: objectId = %d, groupId = %d, pos = %d, My groupid = %d", object->id_, groupId, groupPos, groupId_);
        if (groupId == groupId_)
        {
            AddMember(SharedPtr<Actor>(dynamic_cast<Actor*>(object)), groupPos);
        }
    }
}

bool PartyWindow::IsLeader()
{
    UIElement* elem = memberContainers_[0];
    auto pi = elem->GetChild("HealthBar", true);
    if (pi)
    {
        PartyItem* item = dynamic_cast<PartyItem*>(pi);
        if (item)
            return item->GetActor()->id_ == player_->id_;
    }
    return false;
}
