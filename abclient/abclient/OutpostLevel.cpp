#include "stdafx.h"
#include "OutpostLevel.h"
#include "FwClient.h"
#include "AbEvents.h"
#include "LevelManager.h"

#include <Urho3D/DebugNew.h>

OutpostLevel::OutpostLevel(Context* context) :
    WorldLevel(context)
{
    LevelManager* lm = context_->GetSubsystem<LevelManager>();
    mapName_ = lm->GetMapName();
    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();

    // Subscribe to global events for camera movement
    SubscribeToEvents();

    VariantMap& eData = GetEventDataMap();
    eData[AbEvents::E_LEVEL_READY] = "OutpostLevel";
    SendEvent(AbEvents::E_LEVEL_READY, eData);

    chatWindow_->AddLine("Entered " + mapName_);
}

void OutpostLevel::SubscribeToEvents()
{
    WorldLevel::SubscribeToEvents();
}

void OutpostLevel::CreateUI()
{
    uiRoot_->RemoveAllChildren();
    WorldLevel::CreateUI();
}

void OutpostLevel::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    scene_ = new Scene(context_);
    XMLFile *sceneFile = cache->GetResource<XMLFile>("Scenes/World.xml");
    scene_->LoadXML(sceneFile->GetRoot());
}
