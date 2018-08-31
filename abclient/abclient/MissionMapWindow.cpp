#include "stdafx.h"
#include "MissionMapWindow.h"
#include "Shortcuts.h"
#include "AbEvents.h"
#include "FwClient.h"

void MissionMapWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<MissionMapWindow>();
}

MissionMapWindow::MissionMapWindow(Context* context) :
    Window(context)
{
    SetName("MissionMapWindow");

    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("UI/MissionMapWindow.xml");
    LoadXML(file->GetRoot());

    // It seems this isn't loaded from the XML file
    SetLayoutMode(LM_VERTICAL);
    SetLayoutBorder(IntRect(4, 4, 4, 4));
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
    Text* caption = dynamic_cast<Text*>(GetChild("CaptionText", true));
    caption->SetText(scs->GetCaption(AbEvents::E_SC_TOGGLEMISSIONMAPWINDOW, "Mission Map", true));

    SetSize(300, 300);
    auto* graphics = GetSubsystem<Graphics>();
    SetPosition(graphics->GetWidth() - GetWidth() - 5, graphics->GetHeight() / 2 - (GetHeight() / 2));

    SetStyleAuto();

    UpdateLayout();

    SubscribeToEvents();
}

void MissionMapWindow::OnDragBegin(const IntVector2& position, const IntVector2& screenPosition,
    int buttons, int qualifiers, Cursor* cursor)
{
    Window::OnDragBegin(position, screenPosition, buttons, qualifiers, cursor);
}

void MissionMapWindow::SubscribeToEvents()
{
    Button* closeButton = dynamic_cast<Button*>(GetChild("CloseButton", true));
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(MissionMapWindow, HandleCloseClicked));
}

void MissionMapWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}
