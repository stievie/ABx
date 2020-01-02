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
#pragma once
#include <Urho3D/UI/BorderImage.h>
//#include <Urho3D/UI/CheckBox.h>

namespace Urho3D
{
extern const char* UI_CATEGORY;
}
using namespace Urho3D;
//=============================================================================
//=============================================================================
/// group button released.
URHO3D_EVENT(E_TABSELECTED, TabSelected)
{
    URHO3D_PARAM(P_ELEMENT, Element);              // UIElement pointer
    URHO3D_PARAM(P_INDEX, Index);                  // int
    URHO3D_PARAM(P_OLD_INDEX, OldIndex);           // int
}

struct TabElement
{
    WeakPtr<CheckBox>    tabButton_;
    WeakPtr<Text>        tabText_;
    WeakPtr<BorderImage> tabBody_;
};

//=============================================================================
//=============================================================================
class TabGroup : public BorderImage
{
    URHO3D_OBJECT(TabGroup, BorderImage)
public:
    static void RegisterObject(Context* context);

    TabGroup(Context *context);
    virtual ~TabGroup() override;

    TabElement* CreateTab(const IntVector2 &tabSize, const IntVector2 &bodySize);
    TabElement* GetTabElement(unsigned idx);

    void SetEnabled(bool enabled);

    UIElement* GetHeaderElement() { return headerElement_; }
    UIElement* GetBodyElement()   { return bodyElement_;   }

    int GetSelectedIndex() const { return selectedIndex_; }
    void SetSelectedIndex(int index);

    unsigned int GetTabCount() const { return childList_.Size(); }

protected:
    void HandleTabToggled(StringHash eventType, VariantMap& eventData);
    void SendTabSelectedEvent(int idx, int oldIdx);

protected:
    WeakPtr<UIElement> headerElement_;
    WeakPtr<UIElement> bodyElement_;

    IntVector2         internalSize_;
    Vector<TabElement> childList_;
    int selectedIndex_;
};


