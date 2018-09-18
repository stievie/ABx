#include "stdafx.h"
#include "MapWindow.h"
#include "FwClient.h"
#include "LevelManager.h"

MapWindow::MapWindow(Context* context) :
    Window(context),
    scale_(1.0f)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    mapTexture_ = cache->GetResource<Texture2D>("Textures/world_map.png");
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

    mapSprite_ = CreateChild<Sprite>();

    // Set map sprite texture
    mapSprite_->SetTexture(mapTexture_);
    mapSprite_->SetPosition(0, 0);
    mapSprite_->SetFullImageRect();

    FwClient* client = context_->GetSubsystem<FwClient>();
    const std::map<std::string, AB::Entities::Game>& games = client->GetOutposts();
    int i = 0;
    for (const auto& game : games)
    {
        Button* button = new Button(context_);
        button->SetMinHeight(40);
        button->SetMinWidth(40);
        button->SetMaxHeight(40);
        button->SetMaxWidth(40);
        button->SetHeight(40);
        button->SetWidth(40);
        button->SetName(String(game.first.c_str()));    // not required
        button->SetOpacity(1.0f);     // transparency
        button->SetLayoutMode(LM_FREE);
        button->SetAlignment(HA_LEFT, VA_TOP);
        button->SetPosition(100, 40 * (i + 1) + 5);
        button->SetStyleAuto();
        button->SetStyle("MapButton");
        button->SetVar("uuid", String(game.first.c_str()));
        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(MapWindow, HandleMapGameClicked));
        {
            // buttons don't have a text by itself, a text needs to be added as a child
            Text* t = new Text(context_);
            t->SetName("GameName");
            t->SetText(String(game.second.name.c_str()));
            t->SetStyle("Text");
            t->SetAlignment(HA_LEFT, VA_TOP);
            t->SetPosition(35, 20);
            t->SetInternal(true);
            t->SetStyleAuto();
            button->AddChild(t);
        }
        AddChild(button);
        i++;
    }
    FitMap();

    SubscribeToEvent(E_SCREENMODE, URHO3D_HANDLER(MapWindow, HandleScreenMode));
}

MapWindow::~MapWindow()
{
}

void MapWindow::HandleMapGameClicked(StringHash, VariantMap& eventData)
{
    SetVisible(false);
    Button* sender = static_cast<Button*>(eventData[Urho3D::Released::P_ELEMENT].GetPtr());
    const String uuid = sender->GetVar("uuid").GetString();

    LevelManager* lm = GetSubsystem<LevelManager>();
    if (lm->GetMapUuid().Compare(uuid) != 0)
    {
        FwClient* net = context_->GetSubsystem<FwClient>();
        net->ChangeWorld(uuid);
    }
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
    float scaleX = (float)mapTexture_->GetWidth() / (float)windowWidth;
    float scaleY = (float)mapTexture_->GetHeight() / (float)windowHeight;
    scale_ = Max(scaleX, scaleY);
    mapSprite_->SetSize(static_cast<int>((float)mapTexture_->GetWidth() / scale_),
        static_cast<int>((float)mapTexture_->GetHeight() / scale_));

    int x = windowWidth / 2 - mapSprite_->GetWidth() / 2;
    int y = windowHeight / 2 - mapSprite_->GetHeight() / 2;
    mapSprite_->SetPosition(static_cast<float>(x), static_cast<float>(y));

    SetButtonsPos();
}

void MapWindow::SetButtonsPos()
{
    FwClient* client = context_->GetSubsystem<FwClient>();
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
            float x = (float)game.mapCoordX / scale_ + sPos.x_;
            float y = (float)game.mapCoordY / scale_ + sPos.y_;
            child->SetPosition(static_cast<int>(x) - 20, static_cast<int>(y) - 20);
        }
    }
}
