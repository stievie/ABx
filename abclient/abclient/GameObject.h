#pragma once

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
    Node* objectNode_;
    ObjectType objectType_;
};

