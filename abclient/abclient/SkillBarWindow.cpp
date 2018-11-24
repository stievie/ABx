#include "stdafx.h"
#include "SkillBarWindow.h"

void SkillBarWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<SkillBarWindow>();
}

SkillBarWindow::SkillBarWindow(Context* context) :
    Window(context)
{
    SetName("SkillBar");
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    XMLFile *file = cache->GetResource<XMLFile>("UI/SkillBarWindow.xml");
    LoadChildXML(file->GetRoot(), nullptr);

    // It seems this isn't loaded from the XML file
    SetLayoutMode(LM_VERTICAL);
    SetLayoutBorder(IntRect(4, 4, 4, 4));
    SetPivot(0, 0);
    SetOpacity(0.9f);
    SetResizable(false);
    SetMovable(false);
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    SetTexture(tex);
    SetImageRect(IntRect(48, 0, 64, 16));
    SetBorder(IntRect(4, 4, 4, 4));
    SetImageBorder(IntRect(0, 0, 0, 0));
    SetResizeBorder(IntRect(8, 8, 8, 8));

    SetSize(400, 50);

    auto* graphics = GetSubsystem<Graphics>();
    SetPosition(graphics->GetWidth() / 2 - GetWidth() / 2, graphics->GetHeight() - GetHeight());
    SetVisible(true);

    SetStyleAuto();

    SubscribeEvents();
}

SkillBarWindow::~SkillBarWindow()
{
    UnsubscribeFromAllEvents();
}

void SkillBarWindow::SubscribeEvents()
{
}
