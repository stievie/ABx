#include "stdafx.h"
#include "OutpostLevel.h"
#include "FwClient.h"
#include <sstream>
#include "AbEvents.h"

OutpostLevel::OutpostLevel(Context* context) :
    WorldLevel(context)
{
    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();

    // Subscribe to global events for camera movement
    SubscribeToEvents();

    VariantMap& eData = GetEventDataMap();
    eData[AbEvents::E_LEVEL_READY] = "OutpostLevel";
    SendEvent(AbEvents::E_LEVEL_READY, eData);
}

void OutpostLevel::SubscribeToEvents()
{
    WorldLevel::SubscribeToEvents();
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(OutpostLevel, HandleUpdate));
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

void OutpostLevel::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    FwClient* c = context_->GetSubsystem<FwClient>();
    std::stringstream s;
    s << "Avg. Ping " << c->GetAvgPing() << " Last Ping " << c->GetLastPing();
    pingLabel_->SetText(String(s.str().c_str()));
}
