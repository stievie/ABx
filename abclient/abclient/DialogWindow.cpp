#include "stdafx.h"
#include "DialogWindow.h"

DialogWindow::DialogWindow(Context* context) :
    Window(context)
{
    UI* ui = GetSubsystem<UI>();
    uiRoot_ = ui->GetRoot();
}

DialogWindow::~DialogWindow()
{
    UnsubscribeFromAllEvents();
}

void DialogWindow::SubscribeEvents()
{
    Button* closeButton = dynamic_cast<Button*>(GetChild("CloseButton", true));
    if (closeButton)
        SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(DialogWindow, HandleCloseClicked));
}

void DialogWindow::LoadLayout(const String& fileName)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    XMLFile* file = cache->GetResource<XMLFile>(fileName);
    LoadXML(file->GetRoot());

    // It seems this isn't loaded from the XML file
    SetLayoutMode(LM_VERTICAL);
    SetLayoutBorder(IntRect(4, 4, 4, 4));
    SetPivot(0, 0);
    SetOpacity(0.9f);
    SetResizable(false);
    SetMovable(true);
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    SetTexture(tex);
    SetImageRect(IntRect(48, 0, 64, 16));
    SetBorder(IntRect(4, 4, 4, 4));
    SetImageBorder(IntRect(0, 0, 0, 0));
    SetResizeBorder(IntRect(8, 8, 8, 8));
    SetBringToFront(true);
    SetBringToBack(true);
}

void DialogWindow::Center()
{
    auto* graphics = GetSubsystem<Graphics>();
    SetPosition((graphics->GetWidth() / 2) - (GetWidth() / 2), (graphics->GetHeight() / 2) - (GetHeight() / 2));
}

void DialogWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}
