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

class DamageWindowItem final : public UIElement
{
    URHO3D_OBJECT(DamageWindowItem, UIElement)
private:
    SharedPtr<Text> text_;
public:
    static const int ICON_SIZE = 32;
    DamageWindowItem(Context* context);
    ~DamageWindowItem() override;
    bool Initialize();
    void Add();
    unsigned count_{ 0 };
    int64_t damageTick_{ 0 };
    uint32_t index_{ 0 };
    unsigned value_{ 0 };
};

class DamageWindow final : public Window
{
    URHO3D_OBJECT(DamageWindow, Window)
private:
    static const int KEEP_ITEMS_MS = 5000;
    Vector<SharedPtr<DamageWindowItem>> items_;
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleObjectDamaged(StringHash eventType, VariantMap& eventData);
    DamageWindowItem* FindItem(uint32_t index);
public:
    static void RegisterObject(Context* context);

    DamageWindow(Context* context);
    ~DamageWindow() override;
    void Clear();
};
