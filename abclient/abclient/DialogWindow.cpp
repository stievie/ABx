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
#include "DialogWindow.h"

DialogWindow::DialogWindow(Context* context) :
    Window(context)
{
    UI* ui = GetSubsystem<UI>();
    uiRoot_ = ui->GetRoot();
    uiRoot_->AddChild(this);
}

DialogWindow::~DialogWindow()
{
    UnsubscribeFromAllEvents();
}

void DialogWindow::Close()
{
    SetVisible(false);
    if (overlay_)
        overlay_->Remove();
}

void DialogWindow::SubscribeEvents()
{
    Button* closeButton = GetChildStaticCast<Button>("CloseButton", true);
    if (closeButton)
        SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(DialogWindow, HandleCloseClicked));
}

void DialogWindow::LoadLayout(const String& fileName)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    XMLFile* file = cache->GetResource<XMLFile>(fileName);
    LoadXML(file->GetRoot());

    // It seems this isn't loaded from the XML file
    SetLayoutMode(LM_VERTICAL);
    SetLayoutBorder(IntRect(4, 4, 4, 4));
    SetPivot(0, 0);
    SetOpacity(0.9f);
    SetResizable(false);
    SetMovable(true);
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    SetTexture(tex);
    SetImageRect(IntRect(48, 0, 64, 16));
    SetBorder(IntRect(4, 4, 4, 4));
    SetImageBorder(IntRect(0, 0, 0, 0));
    SetResizeBorder(IntRect(8, 8, 8, 8));
}

void DialogWindow::Center()
{
    auto* graphics = GetSubsystem<Graphics>();
    SetPosition((graphics->GetWidth() / 2) - (GetWidth() / 2), (graphics->GetHeight() / 2) - (GetHeight() / 2));
}

void DialogWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    Close();
}

Window* DialogWindow::EnsureOverlay()
{
    // Ugly Hack to make a pseudo modal window :/
    if (!overlay_)
    {
        auto* ui = GetSubsystem<UI>();
        UIElement* root = ui->GetRoot();

        overlay_ = root->CreateChild<Window>();

        auto* graphics = GetSubsystem<Graphics>();
        overlay_->SetSize(graphics->GetWidth() * 2, graphics->GetHeight() * 2);
        overlay_->SetLayoutMode(LM_FREE);
        overlay_->SetAlignment(HA_LEFT, VA_TOP);
        // Black color
        overlay_->SetColor(Color(0.0f, 0.0f, 0.0f, 1.0f));
        overlay_->SetOpacity(0.5f);
        overlay_->SetPriority(100);
        // Make it top most
        overlay_->SetBringToBack(false);
    }
    overlay_->BringToFront();
    return overlay_.Get();
}
