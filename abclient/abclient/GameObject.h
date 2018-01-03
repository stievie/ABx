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
public:
    GameObject(Context* context);
    ~GameObject();

    /// Initialize the vehicle. Create rendering and physics components. Called by the application.
    virtual void Init() {}

    uint32_t id_;
    unsigned index_;
    ObjectType objectType_;
    bool hovered_;
    AB::GameProtocol::CreatureState creatureState_;

    virtual void Unserialize(PropReadStream& data) {}

    virtual void SetYRotation(float rad, bool updateYaw);
    float GetYRotation();
    virtual void MoveTo(const Vector3& newPos);
    bool IsSelectable() const { return objectType_ > ObjectTypeStatic; }
    IntVector2 WorldToScreenPoint();
    IntVector2 WorldToScreenPoint(Vector3 pos);
    virtual void HoverBegin() { hovered_ = true; }
    virtual void HoverEnd() { hovered_ = false; }

    virtual void RemoveFromScene() {}
};

