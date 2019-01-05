#include "stdafx.h"
#include "TargetWindow.h"
#include "Actor.h"

void TargetWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<TargetWindow>();
}

TargetWindow::TargetWindow(Context* context) :
    UIElement(context)
{
    SetName("TargetWindow");
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    XMLFile *chatFile = cache->GetResource<XMLFile>("UI/TargetWindow.xml");
    LoadChildXML(chatFile->GetRoot(), nullptr);

    targetText_ = dynamic_cast<Text*>(GetChild("TargetText", true));
    SetAlignment(HA_CENTER, VA_TOP);
    healthBar_ = dynamic_cast<HealthBarPlain*>(GetChild("TargetHealthBar", true));
    healthBar_->SetStyle("HealthBar");

    Button* clearTarget = dynamic_cast<Button*>(GetChild("ClearTargetButton", true));
    SubscribeToEvent(clearTarget, E_RELEASED, URHO3D_HANDLER(TargetWindow, HandleClearTargetClicked));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(TargetWindow, HandleUpdate));
}

TargetWindow::~TargetWindow()
{
    UnsubscribeFromAllEvents();
}

void TargetWindow::HandleClearTargetClicked(StringHash, VariantMap&)
{
    VariantMap& e = GetEventDataMap();
    SendEvent(E_TARGETWINDOW_UNSELECT, e);
}

void TargetWindow::HandleUpdate(StringHash, VariantMap&)
{
    if (SharedPtr<Actor> a = target_.Lock())
    {
        healthBar_->SetValues(a->stats_.maxHealth, a->stats_.health);
    }
}

void TargetWindow::SetTarget(SharedPtr<Actor> target)
{
    target_ = target;
    if (target.NotNull())
    {
        targetText_->SetText(target->GetClassLevelName());
        SetVisible(true);
    }
    else
    {
        SetVisible(false);
    }
}
