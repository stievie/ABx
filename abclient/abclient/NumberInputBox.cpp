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
#include "NumberInputBox.h"
#include "Item.h"

NumberInputBox::NumberInputBox(Context* context, const String& title) :
    DialogWindow(context)
{
    LoadLayout("UI/NumberInputBox.xml");
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
    SubscribeToEvent(okButton, E_RELEASED, URHO3D_HANDLER(NumberInputBox, HandleOkClicked));
    auto* cancelButton = GetChildDynamicCast<Button>("CancelButton", true);
    SubscribeToEvent(cancelButton, E_RELEASED, URHO3D_HANDLER(NumberInputBox, HandleCancelClicked));
    auto* edit = GetChildDynamicCast<LineEdit>("NumberInputEdit", true);
    SubscribeToEvent(edit, E_TEXTFINISHED, URHO3D_HANDLER(NumberInputBox, HandleEditTextFinished));
    SubscribeToEvent(edit, E_TEXTENTRY, URHO3D_HANDLER(NumberInputBox, HandleEditTextEntry));
    SubscribeToEvent(edit, E_TEXTCHANGED, URHO3D_HANDLER(NumberInputBox, HandleEditTextChanged));
    auto* maxButton = GetChildStaticCast<Button>("MaxButton", true);
    SubscribeToEvent(maxButton, E_RELEASED, URHO3D_HANDLER(NumberInputBox, HandleMaxButtonClicked));
    maxButton->SetVisible(false);

    MakeModal();
    Center();
    BringToFront();
    edit->SetFocus(true);
}

NumberInputBox::~NumberInputBox()
{ }

void NumberInputBox::SetMax(int value)
{
    auto* button = GetChildStaticCast<Button>("MaxButton", true);
    max_ = value;
    auto* buttonText = button->GetChildStaticCast<Text>("MaxButtonText", true);
    String text;
    text.AppendWithFormat("Max. %s", FormatMoney(max_).CString());
    buttonText->SetText(text);
}

void NumberInputBox::SetMin(int value)
{
    min_ = value;
}

void NumberInputBox::SetShowMaxButton(bool value)
{
    auto* button = GetChildStaticCast<Button>("MaxButton", true);
    button->SetVisible(value);
    haveMax_ = value;
}

void NumberInputBox::SetValue(int value)
{
    auto* edit = GetChildDynamicCast<LineEdit>("NumberInputEdit", true);
    edit->SetText(String(value));
}

int NumberInputBox::GetValue() const
{
    auto* edit = GetChildDynamicCast<LineEdit>("NumberInputEdit", true);
    const String value = edit->GetText();
    if (value.Empty())
        return -1;

    const char* pVal = value.CString();
    char* pEnd;
    return strtol(pVal, &pEnd, 10);
}

void NumberInputBox::SelectAll()
{
    auto* edit = GetChildDynamicCast<LineEdit>("NumberInputEdit", true);
    edit->GetTextElement()->SetSelection(0);
}

void NumberInputBox::HandleOkClicked(StringHash, VariantMap&)
{
    using namespace NumberInputBoxDone;

    auto* edit = GetChildDynamicCast<LineEdit>("NumberInputEdit", true);
    const String value = edit->GetText();
    if (value.Empty())
        return;

    const char* pVal = value.CString();
    char* pEnd;
    int iValue = strtol(pVal, &pEnd, 10);
    if (iValue == 0 || (haveMax_ && iValue > max_) || (HaveMin() && iValue < min_))
        return;

    VariantMap& newEventData = GetEventDataMap();
    newEventData[P_OK] = true;
    newEventData[P_VALUE] = iValue;
    newEventData[P_ELEMENT] = this;
    SendEvent(E_NUMBERINPUTBOXDONE, newEventData);
    Close();
}

void NumberInputBox::HandleCancelClicked(StringHash, VariantMap&)
{
    using namespace NumberInputBoxDone;
    VariantMap& newEventData = GetEventDataMap();
    newEventData[P_OK] = false;
    newEventData[P_VALUE] = 0;
    newEventData[P_ELEMENT] = this;
    SendEvent(E_NUMBERINPUTBOXDONE, newEventData);
    Close();
}

void NumberInputBox::HandleEditTextFinished(StringHash, VariantMap&)
{
    using namespace NumberInputBoxDone;

    auto* edit = GetChildDynamicCast<LineEdit>("NumberInputEdit", true);
    const String value = edit->GetText();
    if (value.Empty())
        return;

    const char* pVal = value.CString();
    char* pEnd;
    int iValue = strtol(pVal, &pEnd, 10);
    if (iValue == 0 || (haveMax_ && iValue > max_) || (HaveMin() && iValue < min_))
        return;

    VariantMap& newEventData = GetEventDataMap();
    newEventData[P_OK] = true;
    newEventData[P_VALUE] = iValue;
    newEventData[P_ELEMENT] = this;
    SendEvent(E_NUMBERINPUTBOXDONE, newEventData);
    Close();
}

void NumberInputBox::HandleEditTextEntry(StringHash, VariantMap& eventData)
{
    using namespace TextEntry;
    String text = eventData[P_TEXT].GetString();
    String newText;
    for (auto it = text.Begin(); it != text.End(); it++)
    {
        if (isdigit(*it))
            newText += (*it);
    }
    eventData[P_TEXT] = newText;
}

void NumberInputBox::HandleEditTextChanged(StringHash, VariantMap& eventData)
{
    using namespace TextChanged;
    auto* okButton = GetChildDynamicCast<Button>("OkButton", true);
    if (eventData[P_TEXT].GetString().Empty())
    {
        okButton->SetEnabled(false);
        return;
    }
    const char* pVal = eventData[P_TEXT].GetString().CString();
    LineEdit* edit = static_cast<LineEdit*>(eventData[P_ELEMENT].GetVoidPtr());
    char* pEnd;
    int iValue = strtol(pVal, &pEnd, 10);
    if (haveMax_)
    {
        if (iValue > max_)
            edit->SetText(String(max_));
    }
    if (HaveMin())
    {
        if (iValue < min_)
            edit->SetText(String(min_));
    }

    okButton->SetEnabled(!edit->GetText().Empty());
}

void NumberInputBox::HandleMaxButtonClicked(StringHash, VariantMap&)
{
    if (!haveMax_)
        return;

    auto* edit = GetChildDynamicCast<LineEdit>("NumberInputEdit", true);
    edit->SetText(String(max_));
}
