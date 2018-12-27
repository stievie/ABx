#pragma once

#include <AB/ProtocolCodes.h>
#include "PropStream.h"

enum ObjectType
{
    ObjectTypeStatic = 0,
    ObjectTypeNpc = 1,
    ObjectTypePlayer = 2,
    ObjectTypeSelf = 3
};

class GameObject : public LogicComponent
{
    URHO3D_OBJECT(GameObject, LogicComponent);
protected:
    AB::GameProtocol::CreatureState creatureState_;
    float speedFactor_;
public:
    GameObject(Context* context);
    ~GameObject() override;

    virtual void Init(Scene*, const Vector3&, const Quaternion&,
        AB::GameProtocol::CreatureState) {}

    uint32_t id_;
    unsigned index_;
    ObjectType objectType_;
    bool undestroyable_;
    int64_t spawnTickServer_;
    /// Player hovers
    bool hovered_;
    /// Player has selected this object
    bool playerSelected_;
    uint32_t groupId_;
    uint8_t groupPos_;
    SharedPtr<SoundSource3D> soundSource_;

    virtual void Unserialize(PropReadStream&) {}

    double GetServerTime(int64_t tick) const
    {
        return (double)(tick - spawnTickServer_) / 1000.0;
    }
    double GetClientTime() const;

    virtual void SetYRotation(float rad, bool updateYaw);
    virtual void SetCreatureState(int64_t time, AB::GameProtocol::CreatureState newState);
    AB::GameProtocol::CreatureState GetCreatureState() const
    {
        return creatureState_;
    }
    float GetSpeedFactor() const
    {
        return speedFactor_;
    }
    virtual void SetSpeedFactor(int64_t time, float value);
    virtual void OnSkillError(AB::GameProtocol::SkillError) { }
    virtual void OnEffectAdded(uint32_t, uint32_t) { }
    virtual void OnEffectRemoved(uint32_t) { }

    float GetYRotation() const;
    virtual void MoveTo(int64_t time, const Vector3& newPos);
    bool IsSelectable() const { return objectType_ > ObjectTypeStatic; }
    IntVector2 WorldToScreenPoint();
    IntVector2 WorldToScreenPoint(Vector3 pos);
    virtual void HoverBegin() { hovered_ = true; }
    virtual void HoverEnd() { hovered_ = false; }

    virtual void RemoveFromScene() {}
};

