#pragma once

#pragma warning( push )
#pragma warning( disable : 4100 4305)
#include <Urho3D/Urho3DAll.h>
#pragma warning( pop )
#include <stdint.h>

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
};

