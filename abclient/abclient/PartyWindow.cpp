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

    UpdateCaption();

    memberContainer_ = dynamic_cast<UIElement*>(GetChild("MemberContainer", true));
    partyContainer_ = dynamic_cast<UIElement*>(GetChild("PartyContainer", true));
    inviteContainer_ = dynamic_cast<UIElement*>(GetChild("InviteContainer", true));
    addContainer_ = dynamic_cast<UIElement*>(GetChild("AddContainer", true));
    addPlayerEdit_ = dynamic_cast<LineEdit*>(GetChild("AddPlayerEdit", true));

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
        SubscribeToEvent(AbEvents::E_OBJECTSELECTED, URHO3D_HANDLER(PartyWindow, HandleObjectSelected));
    }
    else
    {
        UnsubscribeFromEvent(addPlayerButton, E_RELEASED);
        UnsubscribeFromEvent(AbEvents::E_OBJECTSELECTED);
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
    hb->SetActor(actor);
    hb->showName_ = true;
    SubscribeToEvent(hb, E_CLICK, URHO3D_HANDLER(PartyWindow, HandleActorClicked));
    if (type == MemberType::Member)
    {
        members_[actor->id_] = actor;
        hb->SetStyle("HealthBar");
    }
    else
    {
        invitees_[actor->id_] = actor;
        hb->SetStyle("HealthBarInvited");
    }

    if (mode_ == PartyWindowMode::ModeOutpost && actor->objectType_ != ObjectTypeSelf)
    {
        Button* kickButton = cont->CreateChild<Button>();
        kickButton->SetSize(20, 20);
        kickButton->SetMaxSize(20, 20);
        kickButton->SetMinSize(20, 20);
        kickButton->SetAlignment(HA_RIGHT, VA_CENTER);
        kickButton->SetStyleAuto();
        kickButton->SetTexture(cache->GetResource<Texture2D>("Textures/Fw-UI-Ex.png"));
        kickButton->SetImageRect(IntRect(96, 0, 112, 16));
        kickButton->SetHoverOffset(0, 16);
        kickButton->SetPressedOffset(16, 0);
    }
    cont->UpdateLayout();

//    memberContainer_->SetHeight(hb->GetHeight() * memberCount_);
//    memberContainer_->SetMaxHeight(hb->GetHeight() * memberCount_);
    partyContainer_->SetMinHeight(memberContainer_->GetHeight() + inviteContainer_->GetHeight() + 25);
    SetMinHeight(partyContainer_->GetHeight() + 33 + 30);
    UpdateCaption();
    UpdateLayout();
}

void PartyWindow::AddMember(SharedPtr<Actor> actor)
{
    AddItem(memberContainer_, actor, MemberType::Member);
}

void PartyWindow::AddInvite(SharedPtr<Actor> actor)
{
    AddItem(inviteContainer_, actor, MemberType::Invitee);
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

void PartyWindow::Clear()
{
    memberContainer_->RemoveAllChildren();
    inviteContainer_->RemoveAllChildren();
    members_.Clear();
    invitees_.Clear();
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
    if (mode_ != PartyWindowMode::ModeOutpost)
        return;
    using namespace AbEvents::PartyInvited;
    uint32_t sourceId = eventData[P_SOURCEID].GetUInt();
    uint32_t targetId = eventData[P_TARGETID].GetUInt();
    LevelManager* lm = GetSubsystem<LevelManager>();
    GameObject* o = lm->GetObjectById(targetId);
    if (o && o->objectType_ == ObjectTypePlayer)
    {
        AddInvite(SharedPtr<Actor>(dynamic_cast<Actor*>(o)));
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
    using namespace AbEvents::PartyAdded;
    uint32_t actorId = eventData[P_PARTYID].GetUInt();
    LevelManager* lm = GetSubsystem<LevelManager>();
    GameObject* o = lm->GetObjectById(actorId);
    if (o && o->objectType_ == ObjectTypePlayer)
    {
        AddMember(SharedPtr<Actor>(dynamic_cast<Actor*>(o)));
    }
}

void PartyWindow::HandlePartyInviteRemoved(StringHash, VariantMap& eventData)
{
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

void PartyWindow::SubscribeEvents()
{
    Button* closeButton = dynamic_cast<Button*>(GetChild("CloseButton", true));
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(PartyWindow, HandleCloseClicked));
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
