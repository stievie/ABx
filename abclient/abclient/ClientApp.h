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

#include "FwClient.h"
#include "Options.h"
#include "LevelManager.h"
#include "OptionsWindow.h"
#include "ItemsCache.h"
#include "Shortcuts.h"
#include "WindowManager.h"
#include "Mumble.h"
#include "AudioManager.h"
#include "SkillManager.h"
#include <Urho3DAll.h>

using namespace Urho3D;

class ClientApp : public Application
{
    URHO3D_OBJECT(ClientApp, Application)
public:
    /// Construct.
    ClientApp(Context* context);

    void Setup() override;

    /// Setup after engine initialization and before running the main loop.
    void Start() override;
    void Stop() override;
#ifdef DEBUG_HUD
    void CreateHUD();
#endif
private:
    SharedPtr<Options> options_;
    SharedPtr<FwClient> client_;
    SharedPtr<Shortcuts> shortcuts_;
    SharedPtr<ItemsCache> itemsCache_;
    SharedPtr<SkillManager> skillsManager_;
    SharedPtr<LevelManager> levelManager_;
    SharedPtr<WindowManager> windowManager_;
    SharedPtr<AudioManager> audioManager_;
    SharedPtr<Mumble> mumble_;
    String exeName_;
    String appPath_;

    void SetWindowTitleAndIcon();
    void SwitchScene(const String& sceneName);
#ifdef DEBUG_HUD
    void HandleToggleDebugHUD(StringHash eventType, VariantMap& eventData);
    void HandleToggleConsole(StringHash eventType, VariantMap& eventData);
#endif
    void HandleToggleOptions(StringHash eventType, VariantMap& eventData);
    void HandleTakeScreenshot(StringHash eventType, VariantMap& eventData);
    void HandleExitProgram(StringHash eventType, VariantMap& eventData);
    void HandleToggleMuteAudio(StringHash eventType, VariantMap& eventData);
};
