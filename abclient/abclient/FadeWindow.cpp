/**
 * Copyright 2020 Stefan Ascher
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

#include "FadeWindow.h"

FadeWindow::FadeWindow(Context* context) :
    Window(context)
{
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    auto* graphics = GetSubsystem<Graphics>();
    SetSize(graphics->GetWidth(), graphics->GetHeight());
    SetLayout(LM_FREE);

    // Center this window in it's parent element.
    SetAlignment(HA_CENTER, VA_CENTER);
    // Black color
    SetColor(Color(0.0f, 0.0f, 0.0f, 1.0f));
    // Make it top most
    SetBringToBack(false);
    BringToFront();
    bgContainer_ = CreateChild<BorderImage>();

    auto* pg = CreateChild<ProgressBar>("ProgressBar");
    pg->SetStyleAuto();
    pg->SetFixedSize({ GetWidth() / 2, 25 });
    pg->SetAlignment(HA_CENTER, VA_CENTER);
    pg->SetRange(1.0f);
    pg->SetValue(0);
    pg->SetPosition({ 0, (GetHeight() / 2) - 50 });
    pg->BringToFront();
    pg->SetVisible(false);
}

FadeWindow::~FadeWindow()
{ }

void FadeWindow::SetBackground(const String& uuid)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* background = cache->GetResource<Texture2D>("Textures/FadeBackgrounds/" + uuid + ".jpg");
    if (background)
    {
        bgContainer_->SetTexture(background);
        bgContainer_->SetFullImageRect();
        FitBackground();
    }
}

void FadeWindow::FitBackground()
{
    if (!bgContainer_->GetTexture())
        return;

    int windowWidth = GetWidth();
    int windowHeight = GetHeight();
    float scaleX = static_cast<float>(bgContainer_->GetTexture()->GetWidth()) / static_cast<float>(windowWidth);
    float scaleY = static_cast<float>(bgContainer_->GetTexture()->GetHeight()) / static_cast<float>(windowHeight);
    float scale = Max(scaleX, scaleY);
    bgContainer_->SetSize(static_cast<int>(static_cast<float>(bgContainer_->GetTexture()->GetWidth()) / scale),
        static_cast<int>(static_cast<float>(bgContainer_->GetTexture()->GetHeight()) / scale));

    int x = windowWidth / 2 - bgContainer_->GetWidth() / 2;
    int y = windowHeight / 2 - bgContainer_->GetHeight() / 2;
    bgContainer_->SetPosition({ x, y });
}

void FadeWindow::SetScene(Scene* scene)
{
    if (scene == scene_)
        return;

    auto* pg = GetChildStaticCast<ProgressBar>("ProgressBar", true);
    if (scene)
    {
        SubscribeToEvent(scene, E_ASYNCLOADPROGRESS, URHO3D_HANDLER(FadeWindow, HandleAsyncLoadProgress));
        SubscribeToEvent(scene, E_ASYNCLOADFINISHED, URHO3D_HANDLER(FadeWindow, HandleAsyncLoadFinished));

        pg->SetVisible(true);
    }
    else
    {
        UnsubscribeFromEvent(E_ASYNCLOADPROGRESS);
        UnsubscribeFromEvent(E_ASYNCLOADFINISHED);
        pg->SetVisible(false);
    }
    scene_ = scene;
}

void FadeWindow::HandleAsyncLoadFinished(StringHash, VariantMap&)
{
    auto* pg = GetChildStaticCast<ProgressBar>("ProgressBar", true);
    pg->SetValue(1.0f);
}

void FadeWindow::HandleAsyncLoadProgress(StringHash, VariantMap& eventData)
{
    using namespace AsyncLoadProgress;
    auto* pg = GetChildStaticCast<ProgressBar>("ProgressBar", true);
    pg->SetValue(eventData[P_PROGRESS].GetFloat());
}
