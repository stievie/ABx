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
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    XMLFile *chatFile = cache->GetResource<XMLFile>("UI/TargetWindow.xml");
    LoadChildXML(chatFile->GetRoot(), nullptr);

    targetText_ = dynamic_cast<Text*>(GetChild("TargetText", true));
    Button* clearTarget = dynamic_cast<Button*>(GetChild("ClearTargetButton", true));
    SubscribeToEvent(clearTarget, E_RELEASED, URHO3D_HANDLER(TargetWindow, HandleClearTargetClicked));
}

TargetWindow::~TargetWindow()
{
    UnsubscribeFromAllEvents();
}

void TargetWindow::HandleClearTargetClicked(StringHash eventType, VariantMap& eventData)
{
    VariantMap& e = GetEventDataMap();
    SendEvent(E_TARGETWINDOW_UNSELECT, e);
}

void TargetWindow::SetTarget(SharedPtr<GameObject> target)
{
    Actor* actor = dynamic_cast<Actor*>(target.Get());
    if (actor)
    {
        target_ = target;
        if (target.NotNull())
        {
            targetText_->SetText(actor->name_);
            SetVisible(true);
        }
        else
        {
            SetVisible(false);
        }
    }
    else
    {
        SetVisible(false);
    }
}
