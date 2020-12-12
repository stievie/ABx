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
#include "ServerEvents.h"
#include <sa/time.h>
#include "FwClient.h"
#include "AudioManager.h"
#include <abshared/Mechanic.h>

inline constexpr int MAP_WIDTH = 512;
inline constexpr int MAP_HEIGHT = 512;
// Pixel per Meter
inline constexpr int SCALE = 5;

const Color MissionMapWindow::SELF_COLOR(0.3f, 1.0f, 0.3f);
const Color MissionMapWindow::ALLY_COLOR(0.0f, 0.7f, 0.0f);
const Color MissionMapWindow::FOE_COLOR(1.0f, 0.0f, 0.0f);
const Color MissionMapWindow::OTHER_COLOR(0.0f, 0.0f, 1.0f);
const Color MissionMapWindow::WAYPOINT_COLOR(0.46f, 0.07f, 0.04f);
const Color MissionMapWindow::PING_COLOR(1.0f, 0.0f, 0.7f);
const Color MissionMapWindow::MARKER_COLOR(0.0f, 0.5f, 0.0f);
const Color MissionMapWindow::AGGRO_RANGE_COLOR(0.9f, 0.2f, 0.2f);
const Color MissionMapWindow::CASTING_RANGE_COLOR(0.2f, 0.9f, 0.2f);

// 12x12
static constexpr const char* DOT_BITMAP = {
    "    ####    "
    "  ########  "
    " ########## "
    " ########## "
    "############"
    "############"
    "############"
    "############"
    " ########## "
    " ########## "
    "  ########  "
    "    ####    "
};

static constexpr const char* WAYPOINT_BITMAP = {
    "            "
    "            "
    "  ########  "
    "  ########  "
    "  ########  "
    "  ########  "
    "  ########  "
    "  ########  "
    "  ########  "
    "  ########  "
    "            "
    "            "
};

static constexpr const char* PING_BITMAP = {
    "##        ##"
    "###      ###"
    " ###    ### "
    "  ###  ###  "
    "   ######   "
    "    ####    "
    "    ####    "
    "   ######   "
    "  ###  ###  "
    " ###    ### "
    "###      ###"
    "##        ##"
};
static constexpr const char* MARKER_BITMAP = {
    "     ##     "
    "     ##     "
    "##   ##  ## "
    "  ## ## ##  "
    "    ####    "
    "############"
    "############"
    "    ####    "
    "  ## ## ##  "
    "##   ##   ##"
    "     ##     "
    "     ##     "
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

void MissionMapWindow::SetScene(SharedPtr<Scene> scene, AB::Entities::GameType gameType, const String& sceneFile)
{
    waypoints_.Clear();
    gameType_ = gameType;
    if (!scene)
        return;

    String minimapFile = sceneFile + ".minimap.png";
    auto* cache = GetSubsystem<ResourceCache>();
    auto* minimapImage = cache->GetResource<Image>(minimapFile);
    if (!minimapImage)
        URHO3D_LOGWARNING("Minimap file not found");

    terrainLayer_ = GetChildStaticCast<BorderImage>("Container", true);
    objectLayer_ = terrainLayer_->GetChildStaticCast<BorderImage>("ObjectLayer", true);
    auto* terrain = scene->GetComponent<Terrain>(true);
    if (terrain && terrain->GetHeightMap())
    {
        terrainSpacing_ = terrain->GetSpacing();
        auto* heightmap = terrain->GetHeightMap();
        heightmapTexture_ = MakeShared<Texture2D>(context_);
        heightmapTexture_->SetSize(heightmap->GetWidth(), heightmap->GetHeight(), Graphics::GetRGBAFormat(), TEXTURE_STATIC);
        if (minimapImage)
            heightmapTexture_->SetData(minimapImage);
        else
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
    objectLayer_->SetTexture(mapTexture_);
    objectLayer_->SetFullImageRect();
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
    SubscribeToEvent(this, E_RESIZED, URHO3D_HANDLER(MissionMapWindow, HandleResized));
    SubscribeToEvent(Events::E_POSITION_PINGED, URHO3D_HANDLER(MissionMapWindow, HandlePositionPinged));
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(MissionMapWindow, HandleMouseDown));
    SubscribeToEvent(Events::E_OBJECTPINGTARGET, URHO3D_HANDLER(MissionMapWindow, HandleTargetPinged));
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
    float x = (world.x_ * SCALE) + ((float)MAP_WIDTH * 0.5f);
    float y = (-world.z_ * SCALE) + ((float)MAP_HEIGHT * 0.5f);
    return { (int)x, (int)y };
}

Vector3 MissionMapWindow::MapToWorld(const IntVector2& map) const
{
    static const Vector3 offset = {
        (float)MAP_WIDTH * 0.5f, 0.0f, (float)MAP_HEIGHT * 0.5f
    };
    const Vector3 scaling = { (float)terrainLayer_->GetSize().x_ / (float)MAP_WIDTH,
        1.0f,
        (float)terrainLayer_->GetSize().y_ / (float)MAP_HEIGHT };

    const Vector3 pos = Vector3((float)map.x_ / scaling.x_, 0.0f, (float)map.y_ / scaling.z_) - offset;

    return pos / SCALE;
}

Vector3 MissionMapWindow::MapToWorldPos(const Vector3& center, const IntVector2& map) const
{
    return center + MapToWorld(map);
}

void MissionMapWindow::DrawPoint(const IntVector2& center, int size, const Color& color)
{
    for (int y = -(size / 2); y < size / 2; ++y)
    {
        for (int x = -(size / 2); x < size / 2; ++x)
        {
            mapImage_->SetPixel(center.x_ + x, center.y_ + y, color);
        }
    }
}

void MissionMapWindow::DrawCircle(const IntVector2& center, float radius, const Color& color)
{
    const float r = radius * SCALE;
    for (float i = 0; i < 360.0f; i += 0.1f)
    {
        int x = (int)(r * cosf(i * (float)M_PI / 180.0f));
        int y = (int)(r * sinf(i * (float)M_PI / 180.0f));

        DrawPoint({ center.x_ + x, center.y_ + y }, 2, color);
    }
}

void MissionMapWindow::DrawObject(const IntVector2& pos, DotType type)
{
    if (pos.x_ < 0 || pos.x_ > MAP_WIDTH || pos.y_ < 0 || pos.y_ > MAP_HEIGHT)
        return;
    const Color* color = nullptr;
    const char* bitmap = DOT_BITMAP;

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
    case DotType::Waypoint:
        color = &WAYPOINT_COLOR;
        bitmap = WAYPOINT_BITMAP;
        break;
    case DotType::PingPos:
        color = &PING_COLOR;
        bitmap = PING_BITMAP;
        break;
    case DotType::Marker:
        color = &MARKER_COLOR;
        bitmap = MARKER_BITMAP;
        break;
    }
    if (!color)
        return;

    for (int y = 0; y < 12; ++y)
    {
        for (int x = 0; x < 12; ++x)
        {
            if (bitmap[y * 12 + x] == '#')
                mapImage_->SetPixel(pos.x_ + x - 6, pos.y_ + y - 6, *color);
        }
    }
}

Player* MissionMapWindow::GetPlayer() const
{
    LevelManager* lm = GetSubsystem<LevelManager>();
    return lm->GetPlayer();
}

void MissionMapWindow::DrawRanges()
{
    if (auto* p = GetPlayer())
    {
        DrawCircle({ MAP_WIDTH / 2, MAP_HEIGHT / 2 }, Game::RANGE_AGGRO, AGGRO_RANGE_COLOR);
        DrawCircle({ MAP_WIDTH / 2, MAP_HEIGHT / 2 }, Game::RANGE_CASTING, CASTING_RANGE_COLOR);
    }
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
        // Draw waypoints at the bottom
        for (const auto& wp : waypoints_)
        {
            const IntVector2 mapPos = WorldToMapPos(center, wp);
            DrawObject(mapPos, DotType::Waypoint);
        }

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

        if (haveMarker_)
        {
            DrawObject(WorldToMapPos(center, marker_), DotType::Marker);
        }

        if (pingTime_ != 0)
        {
            // Show pinged position/target for 2 seconds
            if (sa::time::time_elapsed(pingTime_) < 2000)
            {
                switch (pingType_)
                {
                case PingType::Position:
                    DrawObject(WorldToMapPos(center, pingPos_), DotType::PingPos);
                    break;
                case PingType::Target:
                {
                    if (auto t = target_.Lock())
                    {
                        const Vector3& targetPos = t->GetNode()->GetPosition();
                        DrawObject(WorldToMapPos(center, targetPos), DotType::PingPos);
                    }
                    break;
                }
                default:
                    break;
                }
            }
        }
    }
}

void MissionMapWindow::HandleRenderUpdate(StringHash, VariantMap&)
{
    if (!IsVisible())
        return;
    mapImage_->Clear(Color::TRANSPARENT_BLACK);
    DrawObjects();
    DrawRanges();
    mapTexture_->SetData(mapImage_, true);
}

void MissionMapWindow::HandleUpdate(StringHash, VariantMap&)
{
    if (!AB::Entities::IsBattle(gameType_))
        return;
    auto* p = GetPlayer();
    if (!p)
        return;

    const auto& pos = p->GetNode()->GetPosition();
    if (waypoints_.Empty() ||
        pos.DistanceToPoint(waypoints_.Back()) > 5.0f)
        waypoints_.Push(pos);
}

void MissionMapWindow::HandleResized(StringHash, VariantMap&)
{
    FitTexture();
}

void MissionMapWindow::HandlePositionPinged(StringHash, VariantMap& eventData)
{
    using namespace Events::PositionPinged;
    pingType_ = PingType::Position;
    pingTime_ = sa::time::tick();
    pingerId_ = eventData[P_OBJECTID].GetUInt();
    pingPos_ = eventData[P_POSITION].GetVector3();
    auto* audio = GetSubsystem<AudioManager>();
    audio->PlaySound("Sounds/FX/Ping.wav", SOUND_EFFECT);
}

void MissionMapWindow::HandleMouseDown(StringHash, VariantMap& eventData)
{
    using namespace MouseButtonDown;
    if (eventData[P_BUTTON].GetUInt() != MOUSEB_LEFT)
        return;

    auto* input = GetSubsystem<Input>();
    UI* ui = GetSubsystem<UI>();
    auto* elem = ui->GetElementAt(input->GetMousePosition(), false);
    if (elem == nullptr || elem != objectLayer_.Get())
        return;

    if (!terrainLayer_->IsInside(input->GetMousePosition(), true))
        return;

    if (auto* p = GetPlayer())
    {
        IntVector2 relativePos = input->GetMousePosition() - terrainLayer_->GetScreenPosition();
        relativePos.y_ = terrainLayer_->GetHeight() - relativePos.y_;
        Vector3 worldPos = MapToWorldPos(p->GetNode()->GetPosition(), relativePos);
        auto* client = GetSubsystem<FwClient>();
        client->PingPosition(worldPos);
    }
}

void MissionMapWindow::HandleTargetPinged(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectPingTarget;

    uint32_t targetId = eventData[P_TARGETID].GetUInt();
    auto* lm = GetSubsystem<LevelManager>();
    target_ = lm->GetObject(targetId);
    if (!target_)
        return;

    pingType_ = PingType::Target;
    pingerId_ = eventData[P_OBJECTID].GetUInt();
    pingTime_ = sa::time::tick();
    auto* audio = GetSubsystem<AudioManager>();
    audio->PlaySound("Sounds/FX/Ping.wav", SOUND_EFFECT);
}
