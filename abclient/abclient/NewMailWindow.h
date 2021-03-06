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

#include "MultiLineEdit.h"
#include <Urho3DAll.h>

class NewMailWindow : public Window
{
    URHO3D_OBJECT(NewMailWindow, Window)
public:
    static void RegisterObject(Context* context);

    NewMailWindow(Context* context);
    ~NewMailWindow() override;
    void SetRecipient(const String& value);
    void SetSubject(const String& value);
    const String& GetSubject() const;
    void FocusBody();
    void SetBody(const String& value);
    void FocusRecipient();
    void FocusSubject();
private:
    SharedPtr<MultiLineEdit> mailBody_;
    SharedPtr<LineEdit> recipient_;
    SharedPtr<LineEdit> subject_;

    void SubscribeToEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleSendClicked(StringHash eventType, VariantMap& eventData);
};

