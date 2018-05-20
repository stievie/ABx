#include "stdafx.h"
#include "MapWindow.h"
#include "FwClient.h"

MapWindow::MapWindow(Context* context) :
    Window(context)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Texture2D* mapTexture = cache->GetResource<Texture2D>("Textures/map.jpg");
    if (!mapTexture)
        return;

    Texture2D* cityTexture = cache->GetResource<Texture2D>("Textures/map_city_icon.png");
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
    SetOpacity(0.7f);
    // Make it top most
    SetBringToBack(false);

    mapSprite_ = CreateChild<Sprite>();

    // Set logo sprite texture
    mapSprite_->SetTexture(mapTexture);
    mapSprite_->SetPosition(0, 0);

    int textureWidth = mapSprite_->GetWidth();
    int textureHeight = mapSprite_->GetHeight();

    // Set logo sprite size
    mapSprite_->SetSize(GetSize());

    FwClient* client = context_->GetSubsystem<FwClient>();
    const std::map<std::string, AB::Entities::Game>& games = client->GetOutposts();
    int i = 0;
    for (const auto& game : games)
    {
        Button* button = new Button(context_);
        button->SetMinHeight(40);
        button->SetMinWidth(40);
        button->SetName(String(game.first.c_str()));    // not required
        button->SetOpacity(1.0f);     // transparency
        button->SetLayoutMode(LM_FREE);
        button->SetAlignment(HA_LEFT, VA_TOP);
        button->SetPosition(100, 40 * (i + 1) + 5);
        button->SetTexture(cityTexture);
        button->SetImageRect(IntRect(0, 0, 256, 256));
        button->SetHoverOffset(IntVector2(257, 0));
        button->SetPressedOffset(IntVector2(513, 0));
        button->SetVar("uuid", String(game.first.c_str()));
        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(MapWindow, HandleMapGameClicked));
        {
            // buttons don't have a text by itself, a text needs to be added as a child
            Text* t = new Text(context_);
            t->SetName("GameName");
            t->SetText(String(game.second.name.c_str()));
            t->SetStyle("Text");
            t->SetLayoutMode(LM_FREE);
            t->SetAlignment(HA_LEFT, VA_TOP);
            t->SetPosition(100 + 45, 40 * (i + 1) + 25);
            AddChild(t);
        }
        AddChild(button);
        i++;
    }
    BringToFront();
}

MapWindow::~MapWindow()
{
}

void MapWindow::HandleMapGameClicked(StringHash eventType, VariantMap& eventData)
{
    Button* sender = static_cast<Button*>(eventData[Urho3D::Released::P_ELEMENT].GetPtr());
    FwClient* net = context_->GetSubsystem<FwClient>();
    const String uuid = sender->GetVar("uuid").GetString();

    net->ChangeWorld(uuid);
}
