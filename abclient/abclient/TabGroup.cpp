//
// Copyright (c) 2008-2016 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "stdafx.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/Resource/ResourceCache.h>

#include "TabGroup.h"

//#include <Urho3D/DebugNew.h>

//=============================================================================
//=============================================================================
void TabGroup::RegisterObject(Context* context)
{
    context->RegisterFactory<TabGroup>(UI_CATEGORY);
}

TabGroup::TabGroup(Context *context) :
    BorderImage(context),
    selectedIndex_(-1)
{
    // default settings
    SetLayoutMode(LM_VERTICAL);
    SetLayoutSpacing(0);
    SetClipChildren(true);

    // tab header - buttons container
    headerElement_ = CreateChild<UIElement>();
    headerElement_->SetLayoutMode(LM_HORIZONTAL);
    headerElement_->SetLayoutSpacing(2);

    // tab body
    bodyElement_ = CreateChild<UIElement>();
    bodyElement_->SetLayoutMode(LM_VERTICAL);
}

TabGroup::~TabGroup()
{
    childList_.Clear();
}

TabElement* TabGroup::CreateTab(const IntVector2 &tabSize, const IntVector2 &bodySize)
{
    TabElement tabElement;

    headerElement_->SetMaxHeight(tabSize.y_);

    // button
    tabElement.tabButton_ = headerElement_->CreateChild<CheckBox>();

    tabElement.tabButton_->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());

    if (childList_.Size() == 0)
    {
        tabElement.tabButton_->SetChecked(true);
        selectedIndex_ = 0;
    }
    tabElement.tabButton_->SetStyle("TabButton");
    tabElement.tabButton_->SetFixedSize(tabSize);
    tabElement.tabButton_->SetImageRect(IntRect(208, 64, 224, 80));
    tabElement.tabButton_->SetCheckedOffset(IntVector2(96, 32));


    tabElement.tabText_ = tabElement.tabButton_->CreateChild<Text>();
    tabElement.tabText_->SetAlignment(HA_CENTER, VA_CENTER);

    // body
    tabElement.tabBody_ = bodyElement_->CreateChild<BorderImage>();

    tabElement.tabBody_->SetStyle("TabBody");
    tabElement.tabBody_->SetImageRect(IntRect(192, 80, 208, 96));
    tabElement.tabBody_->SetSize(bodySize);
    tabElement.tabBody_->SetVisible(childList_.Size() == 0);

    childList_.Push(tabElement);

    return &childList_.Back();
}

TabElement* TabGroup::GetTabElement(unsigned idx)
{
    TabElement *element = NULL;

    if (idx < childList_.Size())
        element = &childList_[idx];

    return element;
}

void TabGroup::SetEnabled(bool enabled)
{
    for ( int i = 0; i < (int)childList_.Size(); ++i )
    {
        if (enabled)
        {
            SubscribeToEvent(childList_[i].tabButton_, E_TOGGLED, URHO3D_HANDLER(TabGroup, HandleTabToggled));
        }
        else
        {
            UnsubscribeFromEvent(childList_[i].tabButton_, E_TOGGLED);
        }

    }
}

void TabGroup::SetSelectedIndex(int index)
{
    using namespace Toggled;

    TabElement* cur = GetTabElement(selectedIndex_);
    if (cur)
    {
        cur->tabButton_->SetChecked(false);
    }

    {
        TabElement* newTab = GetTabElement(index);
        if (newTab)
        {
            newTab->tabButton_->SetChecked(true);
        }
    }
}

void TabGroup::HandleTabToggled(StringHash, VariantMap& eventData)
{
    using namespace Toggled;

    CheckBox *element = (CheckBox*)eventData[P_ELEMENT].GetVoidPtr();
    int checkedIdx = -1;

    for ( int i = 0; i < (int)childList_.Size(); ++i )
    {
        CheckBox *chkbox = childList_[i].tabButton_;

        if (chkbox == element)
        {
            checkedIdx = i;
        }

        chkbox->SetCheckedInternal(chkbox == element);
        childList_[i].tabBody_->SetVisible(chkbox == element);
    }

    if (checkedIdx >= 0)
    {
        int oldIdx = selectedIndex_;
        selectedIndex_ = checkedIdx;
        SendTabSelectedEvent(checkedIdx, oldIdx);
    }
}

void TabGroup::SendTabSelectedEvent(int idx, int oldIdx)
{
    using namespace TabSelected;

    VariantMap& eventData = GetEventDataMap();
    eventData[P_ELEMENT] = this;
    eventData[P_INDEX] = idx;
    eventData[P_OLD_INDEX] = oldIdx;
    SendEvent(E_TABSELECTED, eventData);
}
