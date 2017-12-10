// Level manager, fading [artgolf1000](https://urho3d.prophpbb.com/topic2367.html)

#pragma once

#pragma warning( push )
#pragma warning( disable : 4100 4305)
#include <Urho3D/Urho3DAll.h>
#pragma warning( pop )

#include "AbEvents.h"

using namespace Urho3D;

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
    String mapName_;
    /// Name of level we're coming from. Needed to get the spawn point
    String lastLevelName_;
    SharedPtr<Object> level_;
    SharedPtr<Window> fadeWindow_;
    float fadeTime_;
    int fadeStatus_;
    bool drawDebugGeometry_;
    const float MAX_FADE_TIME = 0.7f;

public:
    Object* GetCurrentLevel() const { return level_; }
    const String& GetLevelName() const { return levelName_ ; }
    const String& GetLastLevelName() const { return lastLevelName_; }
    String GetMapName() const { return mapName_; }
    bool GetDrawDebugGeometry() { return drawDebugGeometry_; }
    void SetDrawDebugGeometry(bool draw);
    void ToggleDebugGeometry()
    {
        SetDrawDebugGeometry(!drawDebugGeometry_);
    }
};