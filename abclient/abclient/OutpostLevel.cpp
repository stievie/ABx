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


#include "OutpostLevel.h"
#include "ChatWindow.h"
#include "FwClient.h"
#include "LevelManager.h"
#include "WindowManager.h"
#include "PartyWindow.h"

//#include <Urho3D/DebugNew.h>

OutpostLevel::OutpostLevel(Context* context) :
    WorldLevel(context)
{
    LevelManager* lm = GetSubsystem<LevelManager>();
    mapUuid_ = lm->GetMapUuid();
    mapType_ = lm->GetMapType();
    partySize_ = lm->GetPartySize();
    FwClient* cli = GetSubsystem<FwClient>();
    mapName_ = cli->GetGameName(mapUuid_);
    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();

    // Subscribe to global events for camera movement
    SubscribeToEvents();

    VariantMap& eData = GetEventDataMap();
    using namespace Events::LevelReady;
    eData[P_NAME] = "OutpostLevel";
    eData[P_TYPE] = static_cast<int>(mapType_);
    SendEvent(Events::E_LEVELREADY, eData);

    chatWindow_->AddLine("Entered " + mapName_, "ChatLogServerInfoText");
}

void OutpostLevel::SubscribeToEvents()
{
    WorldLevel::SubscribeToEvents();
}

void OutpostLevel::CreateUI()
{
    uiRoot_->RemoveAllChildren();
    WorldLevel::CreateUI();

    WindowManager* wm = GetSubsystem<WindowManager>();
    partyWindow_.DynamicCast(wm->GetWindow(WINDOW_PARTY));
    uiRoot_->AddChild(partyWindow_);
    partyWindow_->SetMode(PartyWindowMode::ModeOutpost);
}

void OutpostLevel::CreateScene()
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

    InitModelAnimations();
}
