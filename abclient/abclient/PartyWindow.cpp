#include "stdafx.h"
#include "PartyWindow.h"
#include "AbEvents.h"
#include "WorldLevel.h"
#include "LevelManager.h"
#include "FwClient.h"
#include "PartyItem.h"
#include "Shortcuts.h"
#include "Player.h"

void PartyWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<PartyWindow>();
}

PartyWindow::PartyWindow(Context* context) :
    Window(context),
    partySize_(1),
    player_(nullptr),
    leaderId_(0)
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
    leaderId_ = player->id_;
}

void PartyWindow::SetPartySize(uint8_t value)
{
    if (partySize_ != value)
    {
        partySize_ = value;
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
    UIElement* cont = container->CreateChild<UIElement>();
    cont->SetLayoutMode(LM_HORIZONTAL);
    cont->SetName(actor->name_ + String(actor->id_));
    cont->SetVar("ActorID", actor->id_);
    PartyItem* hb = cont->CreateChild<PartyItem>("HealthBar");
    hb->type_ = type;
    hb->SetAlignment(HA_LEFT, VA_TOP);
    cont->SetHeight(hb->GetHeight());
    cont->SetMaxHeight(hb->GetHeight());
    hb->SetActor(actor);
    hb->showName_ = true;
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

    partyContainer_->SetMinHeight(memberContainer_->GetHeight() + inviteContainer_->GetHeight() + 25);
    SetMinHeight(partyContainer_->GetHeight() + invitationContainer_->GetHeight() + 33 + 30);
    UpdateCaption();
    UpdateLayout();
}

void PartyWindow::AddMember(SharedPtr<Actor> actor)
{
    AddItem(memberContainer_, actor, MemberType::Member);
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
    }
}

void PartyWindow::RemoveInvite(uint32_t actorId)
{
    auto item = GetItem(actorId);
    if (item)
    {
        item->GetParent()->Remove();
        invitees_.Erase(actorId);
    }
}

void PartyWindow::RemoveInvitation(uint32_t actorId)
{
    auto item = GetItem(actorId);
    if (item)
    {
        item->GetParent()->Remove();
        invitations_.Erase(actorId);
    }
}

void PartyWindow::Clear()
{
    memberContainer_->RemoveAllChildren();
    inviteContainer_->RemoveAllChildren();
    invitationContainer_->RemoveAllChildren();
    members_.Clear();
    invitees_.Clear();
    invitations_.Clear();
}

void PartyWindow::HandleAddTargetClicked(StringHash, VariantMap&)
{
    if (mode_ != PartyWindowMode::ModeOutpost)
        return;

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
}

void PartyWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}

void PartyWindow::HandleObjectSelected(StringHash, VariantMap& eventData)
{
    // An object was selected in the world
    using namespace AbEvents::ObjectSelected;
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
    leaderId_ = eventData[P_LEADERID].GetUInt();
    LevelManager* lm = GetSubsystem<LevelManager>();
    GameObject* o = lm->GetObjectById(actorId);
    if (o && o->objectType_ == ObjectTypePlayer)
    {
        Actor* actor = dynamic_cast<Actor*>(o);
        actor->partyId_ = eventData[P_PARTYID].GetUInt();
        AddMember(SharedPtr<Actor>(dynamic_cast<Actor*>(o)));
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

}

void PartyWindow::HandlePartyRemoved(StringHash, VariantMap& eventData)
{
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
}

void PartyWindow::HandleRejectInvitationClicked(StringHash, VariantMap& eventData)
{
}

void PartyWindow::HandleKickClicked(StringHash, VariantMap& eventData)
{
}

void PartyWindow::SubscribeEvents()
{
    Button* closeButton = dynamic_cast<Button*>(GetChild("CloseButton", true));
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(PartyWindow, HandleCloseClicked));
    SubscribeToEvent(AbEvents::E_OBJECTSELECTED, URHO3D_HANDLER(PartyWindow, HandleObjectSelected));
    SubscribeToEvent(AbEvents::E_PARTYADDED, URHO3D_HANDLER(PartyWindow, HandlePartyAdded));
    SubscribeToEvent(AbEvents::E_PARTYINVITED, URHO3D_HANDLER(PartyWindow, HandlePartyInvited));
    SubscribeToEvent(AbEvents::E_PARTYINVITEREMOVED, URHO3D_HANDLER(PartyWindow, HandlePartyInviteRemoved));
    SubscribeToEvent(AbEvents::E_PARTYREMOVED, URHO3D_HANDLER(PartyWindow, HandlePartyRemoved));
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

PartyItem* PartyWindow::GetItem(uint32_t actorId)
{
    if (members_.Contains(actorId))
    {
        auto actor = members_[actorId].Lock();
        UIElement* cont = memberContainer_->GetChild(actor->name_ + String(actor->id_));
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
        UIElement* cont = inviteContainer_->GetChild(actor->name_ + String(actor->id_));
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
        UIElement* cont = invitationContainer_->GetChild(actor->name_ + String(actor->id_));
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
        auto pi = dynamic_cast<PartyItem*>(cont->GetChild("HealthBar", false));
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

bool PartyWindow::IsLeader()
{
    if (auto p = player_.Lock())
        return p->id_ == leaderId_;
    return false;
}
