// Level manager, fading [artgolf1000](https://urho3d.prophpbb.com/topic2367.html)

#pragma once

#pragma warning( push )
#pragma warning( disable : 4100 4305)
#include <Urho3D/Urho3DAll.h>
#pragma warning( pop )

//#include "Player.h"

class BaseLevel : public Object
{
    URHO3D_OBJECT(BaseLevel, Object);
public:
    BaseLevel(Context* context) :
        Object(context),
        uiRoot_(GetSubsystem<UI>()->GetRoot()),
        scene_(nullptr),
//        player_(nullptr),
        cameraNode_(nullptr),
        firstPerson_(false),
        debugGeometry_(false)
    {}

    virtual ~BaseLevel()
    {
        UnsubscribeFromAllEvents();
        Dispose();
    }

    virtual void Run();
    virtual void Pause();
protected:
    virtual void CreateUI();
    virtual void Dispose();
    virtual void SubscribeToEvents();
    virtual void SetupViewport();
    virtual void Update(StringHash eventType, VariantMap& eventData);
    virtual void PostUpdate(StringHash eventType, VariantMap & eventData);
    virtual void PostRenderUpdate(StringHash eventType, VariantMap & eventData);

    Urho3D::UIElement* uiRoot_;
    SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
//    SharedPtr<Player> player_;
    bool firstPerson_;
    /// Camera yaw angle.
    float yaw_;
    /// Camera pitch angle.
    float pitch_;

public:
    bool debugGeometry_;
    /// If this level has a player call this to create it
    virtual void CreatePlayer();
    virtual void CreatePlayer(const Vector3& position);
    virtual void CreatePlayer(const Vector3& position, const Quaternion& direction);
    void ToggleDebugGeometry()
    {
        debugGeometry_ = !debugGeometry_;
    }
private:
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
    void HandlePostRenderUpdate(StringHash eventType, VariantMap & eventData);
};