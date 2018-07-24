#pragma once

#include "FwClient.h"
#include "Options.h"
#include "LevelManager.h"
#include "OptionsWindow.h"
#include "ItemsCache.h"

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

    virtual void Setup();

    /// Setup after engine initialization and before running the main loop.
    virtual void Start();
    virtual void Stop();
protected:

private:
    SharedPtr<Options> options_;
    SharedPtr<FwClient> client_;
    SharedPtr<ItemsCache> itemsCache_;
    SharedPtr<LevelManager> levelManager_;
    SharedPtr<OptionsWindow> optionsWindow_;

    void SetWindowTitleAndIcon();
    void SwitchScene(const String& sceneName);
    /// Handle the logic update event.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleGameMenuOptionsClicked(StringHash eventType, VariantMap& eventData);
};
