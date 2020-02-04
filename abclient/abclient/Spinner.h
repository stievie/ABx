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

#pragma once

#include <Urho3DAll.h>

URHO3D_EVENT(E_VALUECHANGED, ValueChanged)
{
    URHO3D_PARAM(P_ELEMENT, Element);              // UIElement pointer
    URHO3D_PARAM(P_VALUE, Value);                  // int
    URHO3D_PARAM(P_OLDVALUE, OldValue);            // int
}

class Spinner : public BorderImage
{
    URHO3D_OBJECT(Spinner, BorderImage)
private:
    int min_{ 0 };
    int max_{ 100 };
    int value_{ 0 };
    int oldValue_{ 0 };
    unsigned step_{ 1 };
    bool canIncrease_{ true };
    bool canDecrease_{ true };
    WeakPtr<LineEdit> edit_;
    SharedPtr<Button> buttonIncrease_;
    SharedPtr<Button> buttonDecrease_;
    void HandleMouseWheel(StringHash eventType, VariantMap& eventData);
    void HandleIncreaseClicked(StringHash eventType, VariantMap& eventData);
    void HandleDecreaseClicked(StringHash eventType, VariantMap& eventData);
    void Validate();
    void SendValueChangedEvent();
public:
    static void RegisterObject(Context* context);

    Spinner(Context* context);
    ~Spinner() override;

    int GetMin() const { return min_; }
    void SetMin(int value);
    int GetMax() const { return max_; }
    void SetMax(int value);
    int GetValue() const { return value_; }
    void SetValue(int value);
    unsigned GetStep() const { return step_; }
    void SetStep(unsigned value) { step_ = value; }
    void Increase();
    void Decrease();
    void SetEdit(SharedPtr<LineEdit> value);
    bool HaveFocus() const;
    void SetCanIncrease(bool value);
    void SetCanDecrease(bool value);
};
