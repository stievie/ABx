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


#include "ItemUIElement.h"
#include "Item.h"

void ItemUIElement::RegisterObject(Context* context)
{
    context->RegisterFactory<ItemUIElement>();
}

ItemUIElement::ItemUIElement(Context* context) :
    Button(context)
{
    SetLayoutMode(LM_FREE);
    CreateToolTip();
}

ItemUIElement::~ItemUIElement()
{
}

void ItemUIElement::CreateToolTip()
{
    auto* tooltip = CreateChild<ToolTip>("Tooltip");
    tooltip->SetLayoutMode(LM_HORIZONTAL);
    Window* ttWindow = tooltip->CreateChild<Window>();
    ttWindow->SetLayoutMode(LM_VERTICAL);
    ttWindow->SetLayoutBorder(IntRect(4, 4, 4, 4));
    ttWindow->SetStyleAuto();
    tooltipLine1_ = ttWindow->CreateChild<Text>();
    tooltipLine1_->SetStyleAuto();
    tooltipLine2_ = ttWindow->CreateChild<Text>();
    tooltipLine2_->SetStyleAuto();

    tooltip->SetStyleAuto();
    tooltip->SetOpacity(0.9f);
    tooltip->SetPosition(IntVector2(-5, -50));
    hasTooltip_ = true;
}

Window* ItemUIElement::GetDragItem(int buttons, const IntVector2& position)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UIElement* root = GetSubsystem<UI>()->GetRoot();

    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    Window* dragItem = root->CreateChild<Window>();
    dragItem->SetLayout(LM_HORIZONTAL);
    dragItem->SetLayoutBorder(IntRect(4, 4, 4, 4));
    dragItem->SetTexture(tex);
    dragItem->SetImageRect(IntRect(48, 0, 64, 16));
    dragItem->SetBorder(IntRect(4, 4, 4, 4));
    dragItem->SetMinSize(GetSize());
    dragItem->SetMaxSize(GetSize());
    BorderImage* icon = dragItem->CreateChild<BorderImage>();
    icon->SetTexture(GetTexture());
    dragItem->SetPosition(GetPosition());
    dragItem->SetVar("Pos", pos_);
    dragItem->SetVar("Index", index_);
    dragItem->SetVar("Count", count_);
    dragItem->SetVar("Value", value_);
    dragItem->SetVar("Stats", stats_);

    dragItem->SetPosition(position - dragItem->GetSize() / 2);

    SetVar("BUTTONS", buttons);
    return dragItem;
}

void ItemUIElement::SetName(const String& value)
{
    if (name_.Compare(value) == 0)
        return;

    name_ = value;
    String text = count_ > 1 ? String(count_) + " " : "";
    text += name_;
    tooltipLine1_->SetText(text);
}

void ItemUIElement::SetIcon(const String& icon)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Texture2D* texture = cache->GetResource<Texture2D>(icon);
    SetTexture(texture);
    SetFullImageRect();
}

void ItemUIElement::SetCount(unsigned value)
{
    if (count_ == value)
        return;

    count_ = value;
    if (count_ > 1)
    {
        Text* count = GetChildDynamicCast<Text>("Count", true);
        if (!count)
        {
            count = CreateChild<Text>("Count");
            count->SetAlignment(HA_LEFT, VA_BOTTOM);
            count->SetPosition(0, 0);
            count->SetSize(10, GetWidth());
            count->SetMinSize(10, GetWidth());
            count->SetStyleAuto();
            count->SetFontSize(9);
        }
        count->SetText(String(count_));
    }
    else
    {
        auto* elem = GetChild("Count", true);
        if (elem)
            elem->Remove();
    }
    String text = count_ > 1 ? String(count_) + " " : "";
    text += name_;
    tooltipLine1_->SetText(text);
    String text2 = FormatMoney(count_ * value_) + " Drachma";
    tooltipLine2_->SetText(text2);
}

void ItemUIElement::SetValue(unsigned value)
{
    if (value == value_)
        return;

    value_ = value;
    String text2 = FormatMoney(count_ * value_) + " Drachma";
    tooltipLine2_->SetText(text2);
}

void ItemUIElement::SetStats(const String& value)
{
    if (stats_.Compare(value) == 0)
        return;

    stats_ = value;
}

void ItemUIElement::SetHasTooltip(bool value)
{
    if (hasTooltip_ == value)
        return;
    hasTooltip_ = value;
    if (hasTooltip_)
        CreateToolTip();
    else
    {
        auto* tooltip = GetChildStaticCast<ToolTip>("Tooltip", true);
        if (tooltip)
            tooltip->Remove();
    }
}
