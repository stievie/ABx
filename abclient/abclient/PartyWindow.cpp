#include "stdafx.h"
#include "PartyWindow.h"
#include "AbEvents.h"
#include "WorldLevel.h"
#include "LevelManager.h"
#include "FwClient.h"

void PartyWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<PartyWindow>();
}

PartyWindow::PartyWindow(Context* context) :
    Window(context)
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
    SetOpacity(0.9);
    SetResizable(true);
    SetMovable(true);
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    SetTexture(tex);
    SetImageRect(IntRect(48, 0, 64, 16));
    SetBorder(IntRect(4, 4, 4, 4));
    SetImageBorder(IntRect(0, 0, 0, 0));
    SetResizeBorder(IntRect(8, 8, 8, 8));

    // TODO: Load size/position from settings
    SetSize(272, 156);
    auto* graphics = GetSubsystem<Graphics>();
    SetPosition(graphics->GetWidth() - GetWidth() - 5, graphics->GetHeight() / 2 - (GetHeight() / 2));
    SetVisible(true);

    SetStyleAuto();

    SubscribeEvents();
}

PartyWindow::~PartyWindow()
{
    UnsubscribeFromAllEvents();
}

void PartyWindow::SetMode(PartyWindowMode mode)
{
    mode_ = mode;
    if (mode == PartyWindowMode::ModeOutpost)
    {
        addPlayerEdit_ = dynamic_cast<LineEdit*>(GetChild("AddPlayerEdit", true));
        Button* addPlayerButton = dynamic_cast<Button*>(GetChild("AddPlayerButton", true));
        SubscribeToEvent(addPlayerButton, E_RELEASED, URHO3D_HANDLER(PartyWindow, HandleAddTargetClicked));
        Button* closeButton = dynamic_cast<Button*>(GetChild("CloseButton", true));
        SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(PartyWindow, HandleCloseClicked));
        SubscribeToEvent(AbEvents::E_OBJECTSELECTED, URHO3D_HANDLER(PartyWindow, HandleObjectSelected));
    }
    else
    {
        auto* addContainer = dynamic_cast<UIElement*>(GetChild("AddContainer", true));
        RemoveChild(addContainer);
        auto* buttonContainer = dynamic_cast<UIElement*>(GetChild("ButtonContainer", true));
        RemoveChild(buttonContainer);
    }
}

void PartyWindow::HandleAddTargetClicked(StringHash eventType, VariantMap& eventData)
{
    if (mode_ != PartyWindowMode::ModeOutpost)
        return;

    uint32_t targetId = addPlayerEdit_->GetVar("ID").GetUInt();
    FwClient* client = GetSubsystem<FwClient>();
    client->PartyInvite(targetId);
}

void PartyWindow::HandleCloseClicked(StringHash eventType, VariantMap& eventData)
{
    SetVisible(false);
}

void PartyWindow::HandleObjectSelected(StringHash eventType, VariantMap& eventData)
{
    if (!addPlayerEdit_)
        return;

    using namespace AbEvents::ObjectSelected;
    uint32_t targetId = eventData[P_TARGETID].GetUInt();

    LevelManager* lm = GetSubsystem<LevelManager>();
    SharedPtr<GameObject> o = lm->GetObjectById(targetId);
    if (o)
    {
        Actor* a = dynamic_cast<Actor*>(o.Get());
        if (a && a->objectType_ == ObjectTypePlayer)
        {
            addPlayerEdit_->SetText(a->name_);
            addPlayerEdit_->SetVar("ID", targetId);
        }
    }
}

void PartyWindow::HandlePartyInvited(StringHash eventType, VariantMap& eventData)
{
}

void PartyWindow::HandlePartyAdded(StringHash eventType, VariantMap& eventData)
{
}

void PartyWindow::HandlePartyInviteRemoved(StringHash eventType, VariantMap& eventData)
{
}

void PartyWindow::HandlePartyRemoved(StringHash eventType, VariantMap& eventData)
{
}

void PartyWindow::SubscribeEvents()
{
    SubscribeToEvent(AbEvents::E_PARTYADDED, URHO3D_HANDLER(PartyWindow, HandlePartyAdded));
    SubscribeToEvent(AbEvents::E_PARTYINVITED, URHO3D_HANDLER(PartyWindow, HandlePartyInvited));
    SubscribeToEvent(AbEvents::E_PARTYINVITEREMOVED, URHO3D_HANDLER(PartyWindow, HandlePartyInviteRemoved));
    SubscribeToEvent(AbEvents::E_PARTYREMOVED, URHO3D_HANDLER(PartyWindow, HandlePartyRemoved));
}
