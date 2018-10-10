#include "stdafx.h"
#include "FriendListWindow.h"
#include "Shortcuts.h"
#include "AbEvents.h"

void FriendListWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<FriendListWindow>();
}

FriendListWindow::FriendListWindow(Context* context) :
    Window(context)
{
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("UI/FriendListWindow.xml");
    LoadXML(file->GetRoot());

    // It seems this isn't loaded from the XML file
    SetLayoutMode(LM_VERTICAL);
    SetLayoutBorder(IntRect(4, 4, 4, 4));
    SetName("FriendListWindow");
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

    Shortcuts* scs = GetSubsystem<Shortcuts>();
    Text* caption = dynamic_cast<Text*>(GetChild("Caption", true));
    caption->SetText(scs->GetCaption(AbEvents::E_SC_TOGGLEFRIENDLISTWINDOW, "Friends", true));

//    memberContainer_ = dynamic_cast<UIElement*>(GetChild("MemberContainer", true));
//    partyContainer_ = dynamic_cast<UIElement*>(GetChild("PartyContainer", true));

    SetSize(272, 128);
    auto* graphics = GetSubsystem<Graphics>();
    SetPosition(5, graphics->GetHeight() / 2 + GetHeight());
    SetVisible(true);

    SetStyleAuto();

    SubscribeEvents();
}

FriendListWindow::~FriendListWindow()
{
    UnsubscribeFromAllEvents();
}

void FriendListWindow::SubscribeEvents()
{
    Button* closeButton = dynamic_cast<Button*>(GetChild("CloseButton", true));
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(FriendListWindow, HandleCloseClicked));
}

void FriendListWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}
