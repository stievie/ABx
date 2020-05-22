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


#include "MapWindow.h"
#include "FwClient.h"
#include "LevelManager.h"

MapWindow::MapWindow(Context* context) :
    Window(context),
    scale_(1.0f),
    zoom_(1.0f)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    mapTexture_ = cache->GetResource<Texture2D>("Textures/Map/MapPlain.png");
    if (!mapTexture_)
        return;

    // Make the window a child of the root element, which fills the whole screen.
    GetSubsystem<UI>()->GetRoot()->AddChild(this);
    SetSize(GetSubsystem<Graphics>()->GetWidth(), GetSubsystem<Graphics>()->GetHeight());
    SetLayout(LM_FREE);
    // Urho has three layouts: LM_FREE, LM_HORIZONTAL and LM_VERTICAL.
    // In LM_FREE the child elements of this window can be arranged freely.
    // In the other two they are arranged as a horizontal or vertical list.

    // Center this window in it's parent element.
    SetAlignment(HA_CENTER, VA_CENTER);
    // Black color
    SetColor(Color::BLACK);
    SetOpacity(0.8f);
    // Make it top most
    SetBringToBack(false);
    SetPriority(100);
    SetFocusMode(FM_NOTFOCUSABLE);

    mapSprite_ = CreateChild<Sprite>();

    // Set map sprite texture
    mapSprite_->SetTexture(mapTexture_);
    mapSprite_->SetPosition(0, 0);
    mapSprite_->SetFullImageRect();

    FwClient* client = GetSubsystem<FwClient>();
    const std::map<std::string, AB::Entities::Game>& games = client->GetOutposts();
    int i = 0;
    for (const auto& game : games)
    {
        Button* button = CreateChild<Button>(String(game.first.c_str()));
        button->BringToFront();
        button->SetPriority(101);
        button->SetFocusMode(FM_NOTFOCUSABLE);
        button->SetMinHeight(BUTTON_SIZE);
        button->SetMinWidth(BUTTON_SIZE);
        button->SetMaxHeight(BUTTON_SIZE);
        button->SetMaxWidth(BUTTON_SIZE);
        button->SetHeight(BUTTON_SIZE);
        button->SetWidth(BUTTON_SIZE);
        button->SetOpacity(1.0f);     // transparency
        button->SetLayoutMode(LM_FREE);
        button->SetAlignment(HA_LEFT, VA_TOP);
        button->SetVar("uuid", String(game.first.c_str()));
        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(MapWindow, HandleMapGameClicked));
        {
            // buttons don't have a text by itself, a text needs to be added as a child
            Text* t = button->CreateChild<Text>("GameName");
            t->SetText(String(game.second.name.c_str()));
            t->SetStyle("Text");
            t->SetAlignment(HA_LEFT, VA_TOP);
            t->SetPosition(35, 20);
            t->SetInternal(true);
            t->SetStyleAuto();
        }
        button->SetStyleAuto();
        button->SetStyle("MapButton");
        i++;
    }
    FitMap();

    SubscribeToEvent(E_SCREENMODE, URHO3D_HANDLER(MapWindow, HandleScreenMode));
}

MapWindow::~MapWindow()
{
    UnsubscribeFromAllEvents();
}

void MapWindow::HandleMapGameClicked(StringHash, VariantMap& eventData)
{
    SetVisible(false);
    Button* sender = static_cast<Button*>(eventData[Urho3D::Released::P_ELEMENT].GetPtr());
    const String uuid = sender->GetVar("uuid").GetString();

    LevelManager* lm = GetSubsystem<LevelManager>();
    if (lm->GetMapUuid().Compare(uuid) != 0)
    {
        FwClient* net = GetSubsystem<FwClient>();
        net->ChangeMap(uuid);
    }
}

void MapWindow::HandleClicked(StringHash, VariantMap&)
{
    if (Equals(zoom_, 1.0f))
        zoom_ = 2.0f;
    else
        zoom_ = 1.0f;
    FitMap();
}

void MapWindow::HandleScreenMode(StringHash, VariantMap& eventData)
{
    using namespace ScreenMode;
    int w = eventData[P_WIDTH].GetInt();
    int h = eventData[P_HEIGHT].GetInt();
    SetSize(w, h);
    FitMap();
}

void MapWindow::FitMap()
{
    int windowWidth = GetWidth();
    int windowHeight = GetHeight();
    float scaleX = static_cast<float>(mapTexture_->GetWidth()) / static_cast<float>(windowWidth);
    float scaleY = static_cast<float>(mapTexture_->GetHeight()) / static_cast<float>(windowHeight);
    scale_ = Max(scaleX, scaleY) / zoom_;
    mapSprite_->SetSize(static_cast<int>(static_cast<float>(mapTexture_->GetWidth()) / scale_),
        static_cast<int>(static_cast<float>(mapTexture_->GetHeight()) / scale_));

    int x = windowWidth / 2 - mapSprite_->GetWidth() / 2;
    int y = windowHeight / 2 - mapSprite_->GetHeight() / 2;
    mapSprite_->SetPosition(static_cast<float>(x), static_cast<float>(y));

    SetButtonsPos();
}

void MapWindow::SetButtonsPos()
{
    FwClient* client = GetSubsystem<FwClient>();
    const std::map<std::string, AB::Entities::Game>& games = client->GetOutposts();
    auto children = GetChildren();
    for (const auto& child : children)
    {
        if (child == mapSprite_)
            continue;
        const String uuid = child->GetVar("uuid").GetString();
        auto it = games.find(std::string(uuid.CString()));
        if (it != games.end())
        {
            const AB::Entities::Game& game = (*it).second;
            auto sPos = mapSprite_->GetPosition();
            float x = static_cast<float>(game.mapCoordX) / scale_ + sPos.x_;
            float y = static_cast<float>(game.mapCoordY) / scale_ + sPos.y_;
            child->SetPosition(static_cast<int>(x) - 20, static_cast<int>(y) - 20);
        }
    }
}
