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

#include "BaseLevel.h"

class CreateAccountLevel : public BaseLevel
{
    URHO3D_OBJECT(CreateAccountLevel, BaseLevel)
public:
    /// Construct.
    CreateAccountLevel(Context* context);
    void CreateCamera();
    void ShowError(const String& message, const String& title = "Error") override;
protected:
    void SubscribeToEvents() override;
    void CreateUI() override;
private:
    SharedPtr<LineEdit> nameEdit_;
    SharedPtr<LineEdit> passEdit_;
    SharedPtr<LineEdit> repeatPassEdit_;
    SharedPtr<LineEdit> emailEdit_;
    SharedPtr<LineEdit> accKeyEdit_;
    SharedPtr<Text> accKeyPlaceholder_;
    SharedPtr<Button> button_;
    void DoCreateAccount();
    void DoCancel();
    void CreateScene() override;
    void HandleCreateClicked(StringHash eventType, VariantMap& eventData);
    void HandleCancelClicked(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleKeyUp(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleAccKeyFocused(StringHash eventType, VariantMap& eventData);
    void HandleAccKeyDefocused(StringHash eventType, VariantMap& eventData);
};
