/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
