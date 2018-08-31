#pragma once

#include "FwClient.h"
#include "Options.h"
#include "LevelManager.h"
#include "OptionsWindow.h"
#include "ItemsCache.h"
#include "Shortcuts.h"
#include "WindowManager.h"

using namespace Urho3D;

/// This first example, maintaining tradition, prints a "Hello World" message.
/// Furthermore it shows:
///     - Using the Sample / Application classes, which initialize the Urho3D engine and run the main loop
///     - Adding a Text element to the graphical user interface
///     - Subscribing to and handling of update events
class ClientApp : public Application
{
    URHO3D_OBJECT(ClientApp, Application);
public:
    /// Construct.
    ClientApp(Context* context);

    void Setup() override;

    /// Setup after engine initialization and before running the main loop.
    void Start() override;
    void Stop() override;
protected:

private:
    SharedPtr<Options> options_;
    SharedPtr<FwClient> client_;
    SharedPtr<Shortcuts> shortcuts_;
    SharedPtr<ItemsCache> itemsCache_;
    SharedPtr<LevelManager> levelManager_;
    SharedPtr<WindowManager> windowManager_;

    void SetWindowTitleAndIcon();
    void SwitchScene(const String& sceneName);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleToggleOptions(StringHash eventType, VariantMap& eventData);
    void HandleTakeScreenshot(StringHash eventType, VariantMap& eventData);
    void HandleExitProgram(StringHash eventType, VariantMap& eventData);
};
