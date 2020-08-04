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

class LoginLevel final : public BaseLevel
{
    URHO3D_OBJECT(LoginLevel, BaseLevel)
public:
    /// Construct.
    LoginLevel(Context* context);
    void CreateCamera();
    void CreateEnvironmentsList();
    void ShowError(const String& message, const String& title = "Error") override;
protected:
    void SubscribeToEvents() override;
    void CreateUI() override;
private:
    struct LoginServers
    {
        String name;
        String host;
        uint16_t port;
    };
    Vector<LoginServers> servers_;
    int64_t lastPing_{ 0 };
    bool loggingIn_;
    SharedPtr<LineEdit> nameEdit_;
    SharedPtr<LineEdit> passEdit_;
    SharedPtr<Button> button_;
    SharedPtr<Button> createAccountButton_;
    SharedPtr<DropDownList> environmentsList_;
    SharedPtr<Button> pingDot_;
    SharedPtr<Text> pingText_;
    void PingServers();
    void CreateScene() override;
    void HandleLoginClicked(StringHash eventType, VariantMap& eventData);
    void HandleCreateAccountClicked(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleTextFinished(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleServerPing(StringHash eventType, VariantMap& eventData);
    void DoLogin();
    void SetEnvironment();
    Text* CreateDropdownItem(const String& text, const String& value);
};
