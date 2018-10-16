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
    SubscribeToEvent(AbEvents::E_SC_USESKILL1, URHO3D_HANDLER(PvpCombatLevel, HandleUseSkill));
    SubscribeToEvent(AbEvents::E_SC_USESKILL2, URHO3D_HANDLER(PvpCombatLevel, HandleUseSkill));
    SubscribeToEvent(AbEvents::E_SC_USESKILL3, URHO3D_HANDLER(PvpCombatLevel, HandleUseSkill));
    SubscribeToEvent(AbEvents::E_SC_USESKILL4, URHO3D_HANDLER(PvpCombatLevel, HandleUseSkill));
    SubscribeToEvent(AbEvents::E_SC_USESKILL5, URHO3D_HANDLER(PvpCombatLevel, HandleUseSkill));
    SubscribeToEvent(AbEvents::E_SC_USESKILL6, URHO3D_HANDLER(PvpCombatLevel, HandleUseSkill));
    SubscribeToEvent(AbEvents::E_SC_USESKILL7, URHO3D_HANDLER(PvpCombatLevel, HandleUseSkill));
    SubscribeToEvent(AbEvents::E_SC_USESKILL8, URHO3D_HANDLER(PvpCombatLevel, HandleUseSkill));

    SubscribeToEvent(AbEvents::E_SC_CANCEL, URHO3D_HANDLER(PvpCombatLevel, HandleCancel));
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

void PvpCombatLevel::HandleUseSkill(StringHash eventType, VariantMap&)
{
    FwClient* client = GetSubsystem<FwClient>();
    uint32_t index = 0;
    if (eventType == AbEvents::E_SC_USESKILL1)
        index = 1;
    else if (eventType == AbEvents::E_SC_USESKILL2)
        index = 2;
    else if (eventType == AbEvents::E_SC_USESKILL3)
        index = 3;
    else if (eventType == AbEvents::E_SC_USESKILL4)
        index = 4;
    else if (eventType == AbEvents::E_SC_USESKILL5)
        index = 5;
    else if (eventType == AbEvents::E_SC_USESKILL6)
        index = 6;
    else if (eventType == AbEvents::E_SC_USESKILL7)
        index = 7;
    else if (eventType == AbEvents::E_SC_USESKILL8)
        index = 8;
    else
        return;
    client->UseSkill(index);
}

void PvpCombatLevel::HandleCancel(StringHash, VariantMap&)
{
    FwClient* client = GetSubsystem<FwClient>();
    client->Cancel();
}
