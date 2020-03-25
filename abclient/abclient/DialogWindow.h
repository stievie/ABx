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

URHO3D_EVENT(E_DIALOGCLOSE, DialogClose)
{
    URHO3D_PARAM(P_ELEMENT, Element);
}

class DialogWindow : public Window
{
    URHO3D_OBJECT(DialogWindow, Window)
private:
    SharedPtr<Window> overlay_;
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
protected:
    UIElement* uiRoot_;
    virtual void SubscribeEvents();
    void LoadLayout(const String& fileName);
    void MakeModal();
public:
    DialogWindow(Context* context);
    ~DialogWindow() override;
    virtual void Initialize() { }
    void Close();
    void Center();
};

