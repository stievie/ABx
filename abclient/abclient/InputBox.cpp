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

#include "stdafx.h"
#include "InputBox.h"

InputBox::InputBox(Context* context, const String& title) :
    DialogWindow(context)
{
    LoadLayout("UI/InputBox.xml");
    SetStyleAuto();

    SetSize(360, 160);
    SetMinSize(360, 160);
    SetMaxSize(360, 160);
    SetLayoutSpacing(10);
    SetLayoutBorder({ 10, 10, 10, 10 });
    SetMovable(false);

    auto* caption = GetChildDynamicCast<Text>("Caption", true);
    caption->SetText(title);

    auto* okButton = GetChildDynamicCast<Button>("OkButton", true);
    okButton->SetEnabled(false);
    SubscribeToEvent(okButton, E_RELEASED, URHO3D_HANDLER(InputBox, HandleOkClicked));
    auto* cancelButton = GetChildDynamicCast<Button>("CancelButton", true);
    SubscribeToEvent(cancelButton, E_RELEASED, URHO3D_HANDLER(InputBox, HandleCancelClicked));
    auto* edit = GetChildDynamicCast<LineEdit>("InputEdit", true);
    SubscribeToEvent(edit, E_TEXTFINISHED, URHO3D_HANDLER(InputBox, HandleEditTextFinished));
    SubscribeToEvent(edit, E_TEXTCHANGED, URHO3D_HANDLER(InputBox, HandleEditTextChanged));

    MakeModal();
    Center();
    BringToFront();
    edit->SetFocus(true);
}

InputBox::~InputBox()
{
}

void InputBox::SelectAll()
{
    auto* edit = GetChildDynamicCast<LineEdit>("InputEdit", true);
    edit->GetTextElement()->SetSelection(0);
}

void InputBox::SetValue(const String& value)
{
    auto* edit = GetChildDynamicCast<LineEdit>("InputEdit", true);
    edit->SetText(value);
}

const String& InputBox::GetValue() const
{
    auto* edit = GetChildDynamicCast<LineEdit>("InputEdit", true);
    return edit->GetText();
}

void InputBox::HandleOkClicked(StringHash, VariantMap&)
{
    using namespace InputBoxDone;

    auto* edit = GetChildDynamicCast<LineEdit>("InputEdit", true);
    const String value = edit->GetText();
    if (value.Empty())
        return;

    VariantMap& newEventData = GetEventDataMap();
    newEventData[P_OK] = true;
    newEventData[P_VALUE] = value;
    newEventData[P_ELEMENT] = this;
    SendEvent(E_INPUTBOXDONE, newEventData);
    Close();
}

void InputBox::HandleCancelClicked(StringHash, VariantMap&)
{
    using namespace InputBoxDone;
    VariantMap& newEventData = GetEventDataMap();
    newEventData[P_OK] = false;
    newEventData[P_VALUE] = "";
    newEventData[P_ELEMENT] = this;
    SendEvent(E_INPUTBOXDONE, newEventData);
    Close();
}

void InputBox::HandleEditTextFinished(StringHash, VariantMap&)
{
    using namespace InputBoxDone;

    auto* edit = GetChildDynamicCast<LineEdit>("InputEdit", true);
    const String value = edit->GetText();
    if (value.Empty())
        return;

    VariantMap& newEventData = GetEventDataMap();
    newEventData[P_OK] = true;
    newEventData[P_VALUE] = value;
    newEventData[P_ELEMENT] = this;
    SendEvent(E_INPUTBOXDONE, newEventData);
    Close();
}

void InputBox::HandleEditTextChanged(StringHash, VariantMap& eventData)
{
    using namespace TextChanged;
    auto* okButton = GetChildDynamicCast<Button>("OkButton", true);
    okButton->SetEnabled(!eventData[P_TEXT].GetString().Empty());
}
