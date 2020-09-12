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

#include "UpdateProgressWindow.h"
#include "InternalEvents.h"

UpdateProgressWindow::UpdateProgressWindow(Context* context) :
    Window(context)
{
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    auto* graphics = GetSubsystem<Graphics>();
    SetSize(graphics->GetWidth(), graphics->GetHeight());
    SetLayout(LM_FREE);
    SetOpacity(0.0f);

    // Center this window in it's parent element.
    SetAlignment(HA_CENTER, VA_CENTER);
    // Black color
    SetColor(Color(0.0f, 0.0f, 0.0f, 1.0f));
    // Make it top most
    SetBringToBack(false);
    BringToFront();

    auto* pg = CreateChild<ProgressBar>("ProgressBar");
    pg->SetStyleAuto();
    pg->SetFixedSize({ GetWidth() / 2, 25 });
    pg->SetAlignment(HA_CENTER, VA_CENTER);
    pg->SetRange(1.0f);
    pg->SetValue(0);
    pg->SetPosition({ 0, (GetHeight() / 2) - 50 });
    pg->BringToFront();
    pg->SetVisible(true);
    pg->SetUseDerivedOpacity(false);
    pg->SetOpacity(1.0f);

    auto* button = CreateChild<Button>("CancelButton");
    button->SetStyleAuto();
    button->SetLayoutMode(LM_FREE);
    button->SetFixedSize({ 100, 25 });
    button->SetAlignment(HA_RIGHT, VA_CENTER);
    button->SetPosition({ -25, (GetHeight() / 2) - 50 });
    button->BringToFront();
    button->SetVisible(true);
    button->SetUseDerivedOpacity(false);
    button->SetOpacity(1.0f);
    auto* buttonText = button->CreateChild<Text>();
    buttonText->SetStyleAuto();
    buttonText->SetText("Cancel");
    buttonText->SetAlignment(HA_CENTER, VA_CENTER);

    SubscribeToEvent(Events::E_UPDATEPROGRESS, URHO3D_HANDLER(UpdateProgressWindow, HandleUpdateProgress));
    SubscribeToEvent(Events::E_UPDATEDONE, URHO3D_HANDLER(UpdateProgressWindow, HandleUpdateDone));
    SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(UpdateProgressWindow, HandleCancelClicked));
}

UpdateProgressWindow::~UpdateProgressWindow()
{ }

void UpdateProgressWindow::HandleCancelClicked(StringHash, VariantMap&)
{
    VariantMap& eData = GetEventDataMap();
    SendEvent(Events::E_CANCELUPDATE, eData);
}

void UpdateProgressWindow::HandleUpdateProgress(StringHash, VariantMap& eventData)
{
    using namespace Events::UpdateProgress;
    float percent = (float)eventData[P_PERCENT].GetInt() / 100.0f;
    auto* pg = GetChildStaticCast<ProgressBar>("ProgressBar", true);
    pg->SetValue(percent);
    GetSubsystem<Engine>()->RunFrame();
}

void UpdateProgressWindow::HandleUpdateDone(StringHash, VariantMap&)
{
    auto* pg = GetChildStaticCast<ProgressBar>("ProgressBar", true);
    pg->SetValue(1.0f);
}
