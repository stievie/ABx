#include "stdafx.h"
#include "PvpCombatLevel.h"
#include "FwClient.h"
#include "AbEvents.h"
#include "LevelManager.h"
#include "WindowManager.h"

#include <Urho3D/DebugNew.h>

PvpCombatLevel::PvpCombatLevel(Context* context) :
    WorldLevel(context)
{
    LevelManager* lm = context_->GetSubsystem<LevelManager>();
    mapUuid_ = lm->GetMapUuid();
    FwClient* cli = context_->GetSubsystem<FwClient>();
    mapName_ = cli->GetGameName(mapUuid_);
    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();

    // Subscribe to global events for camera movement
    SubscribeToEvents();

    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::LevelReady;
    eData[P_NAME] = "PvpCombatLevel";
    SendEvent(AbEvents::E_LEVELREADY, eData);

    chatWindow_->AddLine("Entered " + mapName_, "ChatLogServerInfoText");
}

void PvpCombatLevel::SubscribeToEvents()
{
    WorldLevel::SubscribeToEvents();
}

void PvpCombatLevel::CreateUI()
{
    uiRoot_->RemoveAllChildren();
    WorldLevel::CreateUI();

    WindowManager* wm = GetSubsystem<WindowManager>();
    partyWindow_.DynamicCast(wm->GetWindow(WINDOW_PARTY));
    uiRoot_->AddChild(partyWindow_);
    partyWindow_->SetMode(PartyWindowMode::ModeOutpost);
}

void PvpCombatLevel::CreateScene()
{
    WorldLevel::CreateScene();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile *sceneFile = cache->GetResource<XMLFile>("Scenes/" + mapName_ + ".xml");
    if (!sceneFile)
    {
        URHO3D_LOGERRORF("Map %s not found", mapName_.CString());
        ShowError("Map \"" + mapName_ + "\" not found", "Error");
        return;
    }
    scene_->LoadXML(sceneFile->GetRoot());
}
