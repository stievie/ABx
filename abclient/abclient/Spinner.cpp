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


#include "Spinner.h"

static const float DEFAULT_REPEAT_DELAY = 0.4f;
static const float DEFAULT_REPEAT_RATE = 20.0f;

void Spinner::RegisterObject(Context* context)
{
    context->RegisterFactory<Spinner>();
}

Spinner::Spinner(Context* context) :
    BorderImage(context)
{
    SetEditable(false);
    SetFocusMode(FM_FOCUSABLE);
    SetLayoutMode(LM_VERTICAL);

    buttonIncrease_ = CreateChild<Button>("Increase");
    buttonIncrease_->SetInternal(true);
    buttonIncrease_->SetRepeat(DEFAULT_REPEAT_DELAY, DEFAULT_REPEAT_RATE);
    buttonIncrease_->SetFocusMode(FM_NOTFOCUSABLE);
    SubscribeToEvent(buttonIncrease_, E_RELEASED, URHO3D_HANDLER(Spinner, HandleIncreaseClicked));

    buttonDecrease_ = CreateChild<Button>("Decrease");
    buttonDecrease_->SetInternal(true);
    buttonDecrease_->SetRepeat(DEFAULT_REPEAT_DELAY, DEFAULT_REPEAT_RATE);
    buttonDecrease_->SetFocusMode(FM_NOTFOCUSABLE);
    SubscribeToEvent(buttonDecrease_, E_RELEASED, URHO3D_HANDLER(Spinner, HandleDecreaseClicked));

    SubscribeToEvent(E_MOUSEWHEEL, URHO3D_HANDLER(Spinner, HandleMouseWheel));

    UpdateLayout();
}

Spinner::~Spinner()
{
    UnsubscribeFromAllEvents();
}

void Spinner::SetMin(int value)
{
    if (min_ != value)
    {
        min_ = value;
        Validate();
    }
}

void Spinner::SetMax(int value)
{
    if (max_ != value)
    {
        max_ = value;
        Validate();
    }
}

void Spinner::SetValue(int value)
{
    if (value != value_)
    {
        value_ = value;
        Validate();
    }
}

void Spinner::Increase()
{
    if (value_ < max_)
    {
        value_ += step_;
        Validate();
    }
}

void Spinner::Decrease()
{
    if (value_ > min_)
    {
        value_ -= step_;
        Validate();
    }
}

bool Spinner::HaveFocus() const
{
    UI* ui = GetSubsystem<UI>();
    auto* f = ui->GetFocusElement();
    if (f == this)
        return true;
    if (auto e = edit_.Lock())
    {
        if (e.Get() == f)
            return true;
    }
    return false;
}

void Spinner::HandleMouseWheel(StringHash, VariantMap& eventData)
{
    if (!HaveFocus())
        return;

    using namespace MouseWheel;
    int v = eventData[P_WHEEL].GetInt();
    if (v > 0 && canIncrease_)
        Increase();
    else if (v < 0 && canDecrease_)
        Decrease();
}

void Spinner::HandleIncreaseClicked(StringHash, VariantMap&)
{
    Increase();
}

void Spinner::HandleDecreaseClicked(StringHash, VariantMap&)
{
    Decrease();
}

void Spinner::HandleEditTextFinished(StringHash, VariantMap&)
{
    const String value = edit_->GetText();
    if (value.Empty())
        return;

    const char* pVal = value.CString();
    char* pEnd;
    value_ = strtol(pVal, &pEnd, 10);

    Validate();
}

void Spinner::HandleEditTextEntry(StringHash, VariantMap& eventData)
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

void Spinner::HandleEditTextChanged(StringHash, VariantMap&)
{
}

void Spinner::HandleEditDefocused(StringHash, VariantMap&)
{
    const String value = edit_->GetText();
    if (value.Empty())
        return;

    const char* pVal = value.CString();
    char* pEnd;
    value_ = strtol(pVal, &pEnd, 10);
    Validate();
}

void Spinner::SetEdit(SharedPtr<LineEdit> value)
{
    edit_ = value;
    if (edit_)
    {
        SubscribeToEvent(edit_, E_TEXTFINISHED, URHO3D_HANDLER(Spinner, HandleEditTextFinished));
        SubscribeToEvent(edit_, E_TEXTENTRY, URHO3D_HANDLER(Spinner, HandleEditTextEntry));
        SubscribeToEvent(edit_, E_TEXTCHANGED, URHO3D_HANDLER(Spinner, HandleEditTextChanged));
        SubscribeToEvent(edit_, E_DEFOCUSED, URHO3D_HANDLER(Spinner, HandleEditDefocused));
    }
    Validate();
}

void Spinner::SetCanIncrease(bool value)
{
    canIncrease_ = value;
    buttonIncrease_->SetEnabled((value_ < max_) && canIncrease_);
}

void Spinner::SetCanDecrease(bool value)
{
    canDecrease_ = value;
    buttonDecrease_->SetEnabled((value_ > min_) && canDecrease_);
}

void Spinner::Validate()
{
    value_ = Clamp(value_, min_, max_);
    if (auto e = edit_.Lock())
        e->SetText(String(value_));
    buttonIncrease_->SetEnabled((value_ < max_) && canIncrease_);
    buttonDecrease_->SetEnabled((value_ > min_) && canDecrease_);

    SendValueChangedEvent();
}

void Spinner::SendValueChangedEvent()
{
    if (oldValue_ == value_)
        return;

    using namespace ValueChanged;
    VariantMap& eventData = GetEventDataMap();
    eventData[P_ELEMENT] = this;
    eventData[P_VALUE] = value_;
    eventData[P_OLDVALUE] = oldValue_;
    oldValue_ = value_;
    SendEvent(E_VALUECHANGED, eventData);
}
