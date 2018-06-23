#pragma once

// Level manager, fading [artgolf1000](https://urho3d.prophpbb.com/topic2367.html)

#include "AbEvents.h"

using namespace Urho3D;

class GameObject;
class Player;

class LevelManager : public Object
{
    URHO3D_OBJECT(LevelManager, Object);
public:
    LevelManager(Context* context);
    ~LevelManager();

private:
    void HandleSetLevelQueue(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void AddFadeLayer();

    List<VariantMap> levelQueue_;
    String levelName_;
    String mapUuid_;
    /// Name of level we're coming from. Needed to get the spawn point
    String lastLevelName_;
    SharedPtr<Object> level_;
    SharedPtr<Window> fadeWindow_;
    float fadeTime_;
    int fadeStatus_;
    bool drawDebugGeometry_;
    const float MAX_FADE_TIME = 0.7f;

public:
    template<typename T>
    T* GetCurrentLevel() const { return dynamic_cast<T*>(level_.Get()); }
    const String& GetLevelName() const { return levelName_ ; }
    const String& GetLastLevelName() const { return lastLevelName_; }
    String GetMapUuid() const { return mapUuid_; }
    bool GetDrawDebugGeometry() { return drawDebugGeometry_; }
    void SetDrawDebugGeometry(bool draw);
    void ToggleDebugGeometry()
    {
        SetDrawDebugGeometry(!drawDebugGeometry_);
    }
    SharedPtr<GameObject> GetObjectById(uint32_t objectId);
    SharedPtr<Player> GetPlayer();
};