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
