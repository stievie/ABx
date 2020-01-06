#include "stdafx.h"
#include "GameMessagesWindow.h"


void GameMessagesWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<GameMessagesWindow>();
}

GameMessagesWindow::GameMessagesWindow(Context* context) :
    UIElement(context),
    visibleTime_(0.0f)
{
    SetName("GameMessagesWindow");
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    XMLFile *chatFile = cache->GetResource<XMLFile>("UI/GameMessagesWindow.xml");
    LoadChildXML(chatFile->GetRoot(), nullptr);

    text_ = GetChildStaticCast<Text>("GameMessageText", true);

    SetAlignment(HA_CENTER, VA_CENTER);
    SetPosition(0, -60);
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(GameMessagesWindow, HandleUpdate));
    SetVisible(false);
}

GameMessagesWindow::~GameMessagesWindow()
{
    UnsubscribeFromAllEvents();
}

void GameMessagesWindow::ShowError(const String& message)
{
    visibleTime_ = 0.0f;
    text_->SetText(message);
    text_->SetStyle("GameMessageError");
    SetVisible(true);
    BringToFront();
}

void GameMessagesWindow::HandleUpdate(StringHash, VariantMap& eventData)
{
    using namespace Update;
    if (IsVisible())
    {
        visibleTime_ += eventData[P_TIMESTEP].GetFloat();
        if (visibleTime_ >= VISIBLE_TIME)
        {
            SetVisible(false);
            visibleTime_ = 0.0f;
        }
    }
}
