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
#include "MissionMapWindow.h"
#include "Shortcuts.h"
#include "LevelManager.h"
#include "Player.h"

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
    int buttons, int qualifiers, Cursor* cursor)
{
    Window::OnDragBegin(position, screenPosition, buttons, qualifiers, cursor);
}

void MissionMapWindow::SetScene(SharedPtr<Scene> scene)
{
    if (!scene)
        return;

    cameraNode_ = scene->CreateChild("MissionMapCamera");
    cameraNode_->Rotate(Quaternion(-90.0f, Vector3::LEFT));
    auto* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(2000.0f);
    camera->SetOrthographic(true);
    camera->SetViewMask(128);

    // Because the rear viewport is rather small, disable occlusion culling from it. Use the camera's
    // "view override flags" for this. We could also disable eg. shadows or force low material quality
    // if we wanted
    camera->SetViewOverrideFlags(VO_DISABLE_OCCLUSION | VO_DISABLE_SHADOWS | VO_LOW_MATERIAL_QUALITY);
    cameraNode_->SetPosition(Vector3(0.0f, 5.0f, 0.0f));

    BorderImage* container = GetChildStaticCast<BorderImage>("Container", true);
    renderTexture_ = new Texture2D(context_);
    renderTexture_->SetSize(4096, 4096, Graphics::GetRGBFormat(), TEXTURE_RENDERTARGET);
    renderTexture_->SetFilterMode(FILTER_BILINEAR);

    container->SetTexture(renderTexture_);
    FitTexture();

    // Get the texture's RenderSurface object (exists when the texture has been created in rendertarget mode)
    // and define the viewport for rendering the second scene, similarly as how backbuffer viewports are defined
    // to the Renderer subsystem. By default the texture viewport will be updated when the texture is visible
    // in the main view
    RenderSurface* surface = renderTexture_->GetRenderSurface();
    SharedPtr<Viewport> rttViewport(new Viewport(context_, scene, cameraNode_->GetComponent<Camera>()));

//    ResourceCache* cache = GetSubsystem<ResourceCache>();
//    SharedPtr<RenderPath> effectRenderPath = rttViewport->GetRenderPath()->Clone();
//    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/GammaCorrection.xml"));
//    effectRenderPath->SetEnabled("GammaCorrection", true);
//    rttViewport->SetRenderPath(effectRenderPath);

//    rttViewport->SetRenderPath(cache->GetResource<XMLFile>("RenderPaths/MiniMap.xml"));
    surface->SetViewport(0, rttViewport);
    surface->SetUpdateMode(SURFACE_UPDATEALWAYS);
}

void MissionMapWindow::FitTexture()
{
    if (!renderTexture_)
        return;

    BorderImage* container = GetChildStaticCast<BorderImage>("Container", true);
/*
    int x = ((renderTexture_->GetWidth() / 2) - (container->GetWidth() / 2));
    int y = ((renderTexture_->GetHeight() / 2) - (container->GetHeight() / 2));
    int width = container->GetWidth() * zoom_;
    int height = container->GetHeight() * zoom_;
    container->SetImageRect(IntRect(x, y, (x + width), (y + height)));
    */
    container->SetImageRect(IntRect(0, 0, renderTexture_->GetWidth(), renderTexture_->GetHeight()));
}

void MissionMapWindow::SubscribeToEvents()
{
    Button* closeButton = GetChildStaticCast<Button>("CloseButton", true);
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(MissionMapWindow, HandleCloseClicked));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MissionMapWindow, HandleUpdate));
    SubscribeToEvent(this, E_RESIZED, URHO3D_HANDLER(MissionMapWindow, HandleResized));
    SubscribeToEvent(this, E_VISIBLECHANGED, URHO3D_HANDLER(MissionMapWindow, HandleVisibleChanged));
}

void MissionMapWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}

void MissionMapWindow::HandleUpdate(StringHash, VariantMap&)
{
    LevelManager* lm = GetSubsystem<LevelManager>();
    auto* p = lm->GetPlayer();
    if (p)
    {
        Vector3 pos = p->GetNode()->GetPosition();
        pos.y_ = 100.0f;
        cameraNode_->SetPosition(pos);
    }
}

void MissionMapWindow::HandleResized(StringHash, VariantMap&)
{
    FitTexture();
}

void MissionMapWindow::HandleVisibleChanged(StringHash, VariantMap& eventData)
{
    if (!renderTexture_)
        return;

    using namespace VisibleChanged;
    bool visible = eventData[P_VISIBLE].GetBool();
    RenderSurface* surface = renderTexture_->GetRenderSurface();
    if (visible)
        surface->SetUpdateMode(SURFACE_UPDATEALWAYS);
    else
        surface->SetUpdateMode(SURFACE_MANUALUPDATE);
}
