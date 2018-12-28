#include "stdafx.h"
#include "PartyWindow.h"
#include "AbEvents.h"
#include "WorldLevel.h"
#include "LevelManager.h"
#include "FwClient.h"
#include "HealthBar.h"
#include "Shortcuts.h"

void PartyWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<PartyWindow>();
}

PartyWindow::PartyWindow(Context* context) :
    Window(context),
    memberCount_(0),
    partySize_(1)
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
    auto* addContainer = dynamic_cast<UIElement*>(GetChild("AddContainer", true));
    auto* buttonContainer = dynamic_cast<UIElement*>(GetChild("ButtonContainer", true));
    Button* addPlayerButton = dynamic_cast<Button*>(GetChild("AddPlayerButton", true));
    if (mode == PartyWindowMode::ModeOutpost)
    {
        addContainer->SetVisible(true);
        buttonContainer->SetVisible(true);
        SubscribeToEvent(addPlayerButton, E_RELEASED, URHO3D_HANDLER(PartyWindow, HandleAddTargetClicked));
        SubscribeToEvent(AbEvents::E_OBJECTSELECTED, URHO3D_HANDLER(PartyWindow, HandleObjectSelected));
    }
    else
    {
        UnsubscribeFromEvent(addPlayerButton, E_RELEASED);
        UnsubscribeFromEvent(AbEvents::E_OBJECTSELECTED);
        addContainer->SetVisible(false);
        buttonContainer->SetVisible(false);
    }
    UpdateCaption();
}

void PartyWindow::AddActor(SharedPtr<Actor> actor)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    UIElement* cont = memberContainer_->CreateChild<UIElement>();
    cont->SetLayoutMode(LM_HORIZONTAL);
    cont->SetVar("ActorID", actor->id_);
    HealthBar* hb = cont->CreateChild<HealthBar>();
    hb->SetAlignment(HA_LEFT, VA_TOP);
    cont->SetHeight(hb->GetHeight());
    hb->SetActor(actor);
    hb->showName_ = true;
    SubscribeToEvent(hb, E_CLICK, URHO3D_HANDLER(PartyWindow, HandleActorClicked));

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

    ++memberCount_;
    memberContainer_->SetHeight(hb->GetHeight() * memberCount_);
    memberContainer_->SetMaxHeight(hb->GetHeight() * memberCount_);
    partyContainer_->SetMinHeight(memberContainer_->GetHeight() + 25);
    SetMinHeight(partyContainer_->GetHeight() + 33 + 30);
    UpdateCaption();
    UpdateLayout();
}

void PartyWindow::Clear()
{
    memberContainer_->RemoveAllChildren();
    memberCount_ = 0;
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
    if (!addPlayerEdit_)
        return;

    // An object was selected in the world
    using namespace AbEvents::ObjectSelected;
    uint32_t targetId = eventData[P_TARGETID].GetUInt();

    if (mode_ != PartyWindowMode::ModeOutpost)
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
        AddActor(SharedPtr<Actor>(dynamic_cast<Actor*>(o)));
    }
}

void PartyWindow::HandlePartyAdded(StringHash, VariantMap& eventData)
{
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
    HealthBar* hb = dynamic_cast<HealthBar*>(eventData[P_ELEMENT].GetPtr());
    SharedPtr<Actor> actor = hb->GetActor();
    if (actor)
    {
        LevelManager* lm = GetSubsystem<LevelManager>();
        Player* player = lm->GetCurrentLevel<BaseLevel>()->GetPlayer();
        player->SelectObject(actor->id_);
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
        s += "  " + String(memberCount_) + "/" + String(static_cast<int>(partySize_));
    caption->SetText(s);
}
