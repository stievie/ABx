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

#include <Urho3DAll.h>
#include "FwClient.h"

class ItemUIElement : public Button
{
    URHO3D_OBJECT(ItemUIElement, Button)
private:
    bool hasTooltip_{ false };
    SharedPtr<Text> tooltipLine1_;
    SharedPtr<Text> tooltipLine2_;
    void CreateToolTip();
public:
    static void RegisterObject(Context* context);
    ItemUIElement(Context* context);
    ~ItemUIElement() override;

    Window* GetDragItem(int buttons, const IntVector2& position);
    void SetName(const String& value);
    void SetPos(unsigned value) { pos_ = value; }
    void SetIndex(unsigned value) { index_ = value; }
    void SetIcon(const String& icon);
    void SetCount(unsigned value);
    void SetValue(unsigned value);
    void SetStats(const String& value);
    void SetHasTooltip(bool value);

    String name_;
    unsigned pos_{ 0 };
    unsigned index_{ 0 };
    unsigned count_{ 0 };
    unsigned value_{ 0 };
    String stats_;
};
