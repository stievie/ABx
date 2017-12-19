#pragma once

#include "BaseLevel.h"
#include "GameObject.h"
#include "ChatWindow.h"
#include <stdint.h>
#include "PropStream.h"
#include "Actor.h"

/// All World maps, Outposts, Combat, Exploreable...
/// These all have the Game UI.
class WorldLevel : public BaseLevel
{
    URHO3D_OBJECT(WorldLevel, BaseLevel);
public:
    WorldLevel(Context* context);
    void CreatePlayer(uint32_t id, const Vector3& position, const Vector3& scale, const Quaternion& direction);
    Actor* CreateActor(uint32_t id, const Vector3& position, const Vector3& scale, const Quaternion& direction);
protected:
    Text* pingLabel_;
    ChatWindow* chatWindow_;
    String mapName_;
    /// All objects in the scene
    HashMap<uint32_t, SharedPtr<GameObject>> objects_;
    void CreateUI() override;
    void SubscribeToEvents() override;
    void Update(StringHash eventType, VariantMap& eventData) override;
    void PostUpdate(StringHash eventType, VariantMap& eventData) override;
private:
    IntVector2 mouseDownPos_;
    bool rmbDown_;
    void HandleMouseDown(StringHash eventType, VariantMap& eventData);
    void HandleMouseUp(StringHash eventType, VariantMap& eventData);
    void HandleMouseWheel(StringHash eventType, VariantMap& eventData);
    void HandleObjectSpawn(StringHash eventType, VariantMap& eventData);
    void HandleObjectDespawn(StringHash eventType, VariantMap& eventData);
    void HandleObjectPosUpdate(StringHash eventType, VariantMap& eventData);
    void HandleObjectRotUpdate(StringHash eventType, VariantMap& eventData);
    void HandleObjectStateUpdate(StringHash eventType, VariantMap& eventData);
    void SpawnObject(uint32_t id, bool existing, const Vector3& position, const Vector3& scale,
        const Quaternion& rot, PropReadStream& data);
};

