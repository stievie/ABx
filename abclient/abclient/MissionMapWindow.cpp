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

const IntRect MissionMapWindow::DOT_NONE(0, 0, 32, 32);
const IntRect MissionMapWindow::DOT_GREEN(32, 0, 64, 32);
const IntRect MissionMapWindow::DOT_ORANGE(64, 0, 96, 32);
const IntRect MissionMapWindow::DOT_RED(96, 0, 128, 32);
inline constexpr int MAP_WIDTH = 512;
inline constexpr int MAP_HEIGHT = 512;

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
    zoom_ = 10;

    UpdateLayout();

    SubscribeToEvents();
}


MissionMapWindow::~MissionMapWindow()
{
    UnsubscribeFromAllEvents();
}

void MissionMapWindow::OnDragBegin(const IntVector2& position, const IntVector2& screenPosition,
    MouseButtonFlags buttons, QualifierFlags qualifiers, Cursor* cursor)
{
    Window::OnDragBegin(position, screenPosition, buttons, qualifiers, cursor);
    // TODO: Move map
}

void MissionMapWindow::SetScene(SharedPtr<Scene> scene)
{
    if (!scene)
        return;

    BorderImage* container = GetChildStaticCast<BorderImage>("Container", true);

    mapTexture_ = MakeShared<Texture2D>(context_);
    mapTexture_->SetSize(MAP_WIDTH, MAP_HEIGHT, Graphics::GetRGBAFormat(), TEXTURE_DYNAMIC);
    mapImage_ = MakeShared<Image>(context_);
    mapImage_->SetSize(MAP_WIDTH, MAP_HEIGHT, 4);
    mapTexture_->SetData(mapImage_);

    auto* cache = GetSubsystem<ResourceCache>();
    dots_ = cache->GetResource<Texture2D>("Textures/PingDot.png");
    greenDot_ = dots_->GetImage()->GetSubimage(MissionMapWindow::DOT_GREEN);

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

IntVector2 MissionMapWindow::WorldToMapPos(const Vector3& center, const Vector3& world) const
{
    Vector3 diff = center - world;
    int x = (int)(diff.x_ / (float)MAP_WIDTH) + MAP_WIDTH / 2;
    int y = (int)(diff.z_ / (float)MAP_HEIGHT) + MAP_HEIGHT / 2;
    return { x, y };
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

void MissionMapWindow::DrawObject(const IntVector2& pos)
{
    if (pos.x_ < 0 || pos.x_ > MAP_WIDTH || pos.y_ < 0 || pos.y_ > MAP_HEIGHT)
        return;
    IntRect dst(pos, pos + MissionMapWindow::DOT_GREEN.Size());
    mapImage_->SetSubimage(greenDot_, dst);
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
        lvl->VisitObjects([this, &center](GameObject& current)
        {
            IntVector2 mapPos = WorldToMapPos(center, current.GetNode()->GetPosition());
            DrawObject(mapPos);
            return Iteration::Continue;
        });
    }
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
