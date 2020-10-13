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

#include "MissionMapWindow.h"
#include "Shortcuts.h"
#include "LevelManager.h"
#include "Player.h"
#include "WorldLevel.h"

inline constexpr int MAP_WIDTH = 512;
inline constexpr int MAP_HEIGHT = 512;
// Pixel per Meter
inline constexpr float SCALE = 5.0f;

const Color MissionMapWindow::SELF_COLOR(0.3f, 1.0f, 0.3f);
const Color MissionMapWindow::ALLY_COLOR(0.0f, 0.7f, 0.0f);
const Color MissionMapWindow::FOE_COLOR(1.0f, 0.0f, 0.0f);
const Color MissionMapWindow::OTHER_COLOR(0.0f, 0.0f, 1.0f);

// 12x12
static const char* DOT_BITMAP = {
    "    ####    "
    "  ########  "
    " ########## "
    " ########## "
    "############"
    "############"
    "############"
    " ########## "
    " ########## "
    "  ########  "
    "    ####    "
};

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
    SetBringToFront(true);
    SetBringToBack(true);

    Shortcuts* scs = GetSubsystem<Shortcuts>();
    Text* caption = GetChildStaticCast<Text>("CaptionText", true);
    caption->SetText(scs->GetCaption(Events::E_SC_TOGGLEMISSIONMAPWINDOW, "Mission Map", true));

    SetSize(300, 300);
    auto* graphics = GetSubsystem<Graphics>();
    SetPosition(graphics->GetWidth() - GetWidth() - 5, graphics->GetHeight() / 2 - (GetHeight() / 2));

    SetStyleAuto();

    UpdateLayout();

    SubscribeToEvents();
}


MissionMapWindow::~MissionMapWindow()
{
    UnsubscribeFromAllEvents();
}

void MissionMapWindow::SetScene(SharedPtr<Scene> scene)
{
    if (!scene)
        return;

    auto* terrain = scene->GetComponent<Terrain>(true);
    if (terrain)
    {
        heightmap_ = terrain->GetHeightMap();
        heightmapMax_ = { ((float)heightmap_->GetWidth() * terrain->GetSpacing().x_), 0.0f,
            ((float)heightmap_->GetHeight() * terrain->GetSpacing().z_) };
    }

    BorderImage* container = GetChildStaticCast<BorderImage>("Container", true);

    mapTexture_ = MakeShared<Texture2D>(context_);
    mapTexture_->SetSize(MAP_WIDTH, MAP_HEIGHT, Graphics::GetRGBAFormat(), TEXTURE_DYNAMIC);
    mapImage_ = MakeShared<Image>(context_);
    mapImage_->SetSize(MAP_WIDTH, MAP_HEIGHT, 4);
    mapTexture_->SetData(mapImage_, true);

    container->SetTexture(mapTexture_);
    container->SetFullImageRect();
}

void MissionMapWindow::FitTexture()
{
    if (!mapTexture_)
        return;

    BorderImage* container = GetChildStaticCast<BorderImage>("Container", true);
    container->SetFullImageRect();
}

void MissionMapWindow::SubscribeToEvents()
{
    Button* closeButton = GetChildStaticCast<Button>("CloseButton", true);
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(MissionMapWindow, HandleCloseClicked));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MissionMapWindow, HandleUpdate));
    SubscribeToEvent(E_RENDERUPDATE, URHO3D_HANDLER(MissionMapWindow, HandleRenderUpdate));
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(MissionMapWindow, HandlePostRenderUpdate));
    SubscribeToEvent(this, E_RESIZED, URHO3D_HANDLER(MissionMapWindow, HandleResized));
}

void MissionMapWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}

IntVector2 MissionMapWindow::WorldToMapPos(const Vector3& center, const Vector3& world) const
{
    Vector3 diff = world - center;
    float x = (diff.x_ * SCALE) + ((float)MAP_WIDTH / 2.0f);
    float y = (-diff.z_ * SCALE) + ((float)MAP_HEIGHT / 2.0f);
    return { (int)x, (int)y };
}

void MissionMapWindow::DrawObject(const IntVector2& pos, DotType type)
{
    if (pos.x_ < 0 || pos.x_ > MAP_WIDTH || pos.y_ < 0 || pos.y_ > MAP_HEIGHT)
        return;
    const Color* color = nullptr;
    switch (type)
    {
    case DotType::Self:
        color = &SELF_COLOR;
        break;
    case DotType::Ally:
        color = &ALLY_COLOR;
        break;
    case DotType::Other:
        color = &OTHER_COLOR;
        break;
    case DotType::Foe:
        color = &FOE_COLOR;
        break;
    }
    if (!color)
        return;

    for (int x = 0; x <= 12; ++x)
    {
        for (int y = 0; y <= 12; ++y)
        {
            if (DOT_BITMAP[y * 12 + x] == '#')
                mapImage_->SetPixel(pos.x_ + x - 6, pos.y_ + y - 6, *color);
        }
    }
}

Player* MissionMapWindow::GetPlayer() const
{
    LevelManager* lm = GetSubsystem<LevelManager>();
    return lm->GetPlayer();
}

void MissionMapWindow::DrawObjects()
{
    auto* lm = GetSubsystem<LevelManager>();
    WorldLevel* lvl = lm->GetCurrentLevel<WorldLevel>();
    if (!lvl)
        return;

    if (auto* p = GetPlayer())
    {
        const Vector3& center = p->GetNode()->GetPosition();
        // TOTO: Draw heightmap as terrain
/*        if (heightmap_)
        {
            IntVector2 min = WorldToMapPos(center, heightmapMin_);
            IntVector2 max = WorldToMapPos(center, heightmapMax_);
            mapImage_->SetSubimage(heightmap_, { min, max });
        }
        else*/
            mapImage_->Clear(Color::TRANSPARENT_BLACK);

        lvl->VisitObjects([this, &center, p](GameObject& current)
        {
            if (!Is<Actor>(current) || !current.IsPlayingCharacterOrNpc())
                return Iteration::Continue;
            const IntVector2 mapPos = WorldToMapPos(center, current.GetNode()->GetPosition());
            DotType type;
            if (current.GetID() == p->GetID())
                type = DotType::Self;
            else if (p->IsAlly(&To<Actor>(current)))
                type = DotType::Ally;
            else if (p->IsEnemy(&To<Actor>(current)))
                type = DotType::Foe;
            else
                type = DotType::Other;
            // TODO: Team color
            DrawObject(mapPos, type);
            return Iteration::Continue;
        });
    }
    else
        mapImage_->Clear(Color::WHITE);
    mapTexture_->SetData(mapImage_, true);
}

void MissionMapWindow::HandleRenderUpdate(StringHash, VariantMap&)
{
    if (!IsVisible())
        return;
    DrawObjects();
}

void MissionMapWindow::HandlePostRenderUpdate(StringHash, VariantMap&)
{
}

void MissionMapWindow::HandleUpdate(StringHash, VariantMap&)
{
}

void MissionMapWindow::HandleResized(StringHash, VariantMap&)
{
    FitTexture();
}
