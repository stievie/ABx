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

// Level manager, fading [artgolf1000](https://urho3d.prophpbb.com/topic2367.html)

#include <AB/Entities/Game.h>
#include <Urho3DAll.h>

using namespace Urho3D;

class GameObject;
class Player;
class Actor;
class FadeWindow;
class UpdateProgressWindow;

class LevelManager : public Object
{
    URHO3D_OBJECT(LevelManager, Object)
public:
    LevelManager(Context* context);
    ~LevelManager() override;
private:
    static constexpr float MAX_FADE_TIME = 0.7f;
    enum FadeStatus
    {
        FadeStatusPrepare = 0,
        FadeStatusFadeOut,
        FadeStatusReleaseOld,
        FadeStatusCreateNew,
        FadeStatusFadeIn,
        FadeStatusFinish
    };
    void HandleSetLevelQueue(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleLevelReady(StringHash eventType, VariantMap& eventData);
    void HandleUpdateStart(StringHash eventType, VariantMap& eventData);
    void HandleUpdateDone(StringHash eventType, VariantMap& eventData);
    void AddFadeLayer();

    List<VariantMap> levelQueue_;
    String levelName_;
    String mapUuid_;
    AB::Entities::GameType mapType_{ AB::Entities::GameTypeUnknown };
    uint8_t partySize_{ 0 };
    /// Name of level we're coming from. Needed to get the spawn point
    String lastLevelName_;
    String instanceUuid_;
    SharedPtr<Object> level_;
    SharedPtr<FadeWindow> fadeWindow_;
    SharedPtr<UpdateProgressWindow> updateWindow_;
    float fadeTime_{ 0 };
    int fadeStatus_{ FadeStatusPrepare };
    bool drawDebugGeometry_{ false };
    bool readyToFadeIn_{ false };
public:
    template<typename T>
    T* GetCurrentLevel() const { return dynamic_cast<T*>(level_.Get()); }
    const String& GetLevelName() const { return levelName_ ; }
    const String& GetLastLevelName() const { return lastLevelName_; }
    const String& GetInstanceUuid() const { return instanceUuid_; }
    String GetMapUuid() const { return mapUuid_; }
    AB::Entities::GameType GetMapType() const { return mapType_; }
    const AB::Entities::Game* GetGame() const;
    uint8_t GetPartySize() const { return partySize_; }
    bool GetDrawDebugGeometry() { return drawDebugGeometry_; }
    void SetDrawDebugGeometry(bool draw);
    void ToggleDebugGeometry()
    {
        SetDrawDebugGeometry(!drawDebugGeometry_);
    }

    GameObject* GetObject(uint32_t objectId);
    Actor* GetActorByName(const String& name) const;
    Player* GetPlayer() const;
    Camera* GetCamera() const;
    void ShowError(const String& message, const String& title);
};
