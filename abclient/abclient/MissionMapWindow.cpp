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
inline constexpr int SCALE = 5;

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

    terrainLayer_ = GetChildStaticCast<BorderImage>("Container", true);
    BorderImage* objectLayer = terrainLayer_->GetChildStaticCast<BorderImage>("ObjectLayer", true);
    auto* terrain = scene->GetComponent<Terrain>(true);
    if (terrain && terrain->GetHeightMap())
    {
        terrainSpacing_ = terrain->GetSpacing();
        auto* heightmap = terrain->GetHeightMap();
        heightmapTexture_ = MakeShared<Texture2D>(context_);
        heightmapTexture_->SetSize(heightmap->GetWidth(), heightmap->GetHeight(), Graphics::GetRGBAFormat(), TEXTURE_STATIC);
        heightmapTexture_->SetData(heightmap, true);
        heightmapTexture_->SetNumLevels(1);
        heightmapTexture_->SetMipsToSkip(QUALITY_LOW, 0);
        terrainLayer_->SetTexture(heightmapTexture_);
        terrainWorldSize_ = { (float)heightmap->GetWidth() * terrainSpacing_.x_,
            terrainSpacing_.y_,
            (float)heightmap->GetHeight() * terrainSpacing_.z_ };
        terrainScaling_ = { terrainWorldSize_.x_ / (float)MAP_WIDTH, terrainWorldSize_.z_ / (float)MAP_HEIGHT };
    }
    else
        terrainLayer_->SetTexture(nullptr);

    mapTexture_ = MakeShared<Texture2D>(context_);
    mapTexture_->SetSize(MAP_WIDTH, MAP_HEIGHT, Graphics::GetRGBAFormat(), TEXTURE_DYNAMIC);
    mapTexture_->SetNumLevels(1);
    mapTexture_->SetMipsToSkip(QUALITY_LOW, 0);
    mapImage_ = MakeShared<Image>(context_);
    mapImage_->SetSize(MAP_WIDTH, MAP_HEIGHT, 4);
    mapTexture_->SetData(mapImage_, true);
    objectLayer->SetTexture(mapTexture_);
    objectLayer->SetFullImageRect();
}

void MissionMapWindow::FitTexture()
{
    if (!mapTexture_)
        return;
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
    return WorldToMap(world - center);
}

IntVector2 MissionMapWindow::WorldToMap(const Vector3& world) const
{
    float x = (world.x_ * SCALE) + ((float)MAP_WIDTH / 2.0f);
    float y = (-world.z_ * SCALE) + ((float)MAP_HEIGHT / 2.0f);
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

    for (int y = 0; y < 12; ++y)
    {
        for (int x = 0; x < 12; ++x)
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

    mapImage_->Clear(Color::TRANSPARENT_BLACK);
    if (auto* p = GetPlayer())
    {
        const Vector3& center = p->GetNode()->GetPosition();
        const IntVector2 origin = WorldToMap(center);

        // TODO: Fix this! Heck that's 3 coordinate systems. You shouldn't make a game when you can't to some math :/
        // However, this is just trying to get some texture displayed as map.
        IntVector2 sourceSize = {
            // (world.x_ * SCALE) + ((float)MAP_WIDTH / 2.0f)
            int((float)MAP_WIDTH / (float)SCALE * terrainScaling_.x_),
            int((float)MAP_HEIGHT / (float)SCALE * terrainScaling_.y_)
        };
        IntRect imageRect = { (origin.x_ - sourceSize.x_),
                (origin.y_ - sourceSize.y_),
                (origin.x_ + sourceSize.x_),
                (origin.y_ + sourceSize.y_) };
        terrainLayer_->SetImageRect(imageRect);
//        terrainLayer_->SetFullImageRect();

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
    mapTexture_->SetData(mapImage_, true);
    //mapImage_->SavePNG("c:/Users/Stefan Ascher/Documents/Visual Studio 2015/Projects/ABx/abclient/bin/test.png");
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
