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

#pragma once

#include "DialogWindow.h"

URHO3D_EVENT(E_NUMBERINPUTBOXDONE, NumberInputBoxDone)
{
    URHO3D_PARAM(P_OK, Ok);              // bool
    URHO3D_PARAM(P_VALUE, Value);              // int
    URHO3D_PARAM(P_ELEMENT, Element);
}

class NumberInputBox : public DialogWindow
{
    URHO3D_OBJECT(NumberInputBox, DialogWindow)
private:
    int max_{ 0 };
    int min_{ -1 };
    bool haveMax_{ false };
    void HandleOkClicked(StringHash eventType, VariantMap& eventData);
    void HandleCancelClicked(StringHash eventType, VariantMap& eventData);
    void HandleEditTextFinished(StringHash eventType, VariantMap& eventData);
    void HandleEditTextEntry(StringHash eventType, VariantMap& eventData);
    void HandleEditTextChanged(StringHash eventType, VariantMap& eventData);
    void HandleMaxButtonClicked(StringHash eventType, VariantMap& eventData);
    bool HaveMin() const { return min_ > -1; }
public:
    NumberInputBox(Context* context, const String& title);
    ~NumberInputBox() override;
    void SetMax(int value);
    int GetMax() const { return max_; }
    void SetMin(int value);
    int GetMin() const { return min_; }
    void SetShowMaxButton(bool value);
    void SetValue(int value);
    int GetValue() const;
    void SelectAll();
};

