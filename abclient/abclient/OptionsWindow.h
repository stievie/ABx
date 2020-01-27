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

#include "TabGroup.h"
#include "Shortcuts.h"

class OptionsWindow : public Window
{
    URHO3D_OBJECT(OptionsWindow, Window)
private:
    SharedPtr<TabGroup> tabgroup_;
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleTabSelected(StringHash eventType, VariantMap& eventData);
    void HandleFovSliderChanged(StringHash eventType, VariantMap& eventData);
    void SubscribeEvents();
    TabElement* CreateTab(TabGroup* tabs, const String& page);
    void CreatePageGeneral(TabElement* tabElement);
    void CreatePageGraphics(TabElement* tabElement);
    void CreatePageAudio(TabElement* tabElement);
    void CreatePageInput(TabElement* tabElement);
    void CreatePageInterface(TabElement* tabElement);
    void LoadWindow(Window* wnd, const String& fileName);
    void FillShortcutsList();
    void HandleShortcutItemSelected(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
public:
    static void RegisterObject(Context* context);

    OptionsWindow(Context* context);
    ~OptionsWindow() override;
};

