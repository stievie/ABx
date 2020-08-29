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

// Level manager, fading [artgolf1000](https://urho3d.prophpbb.com/topic2367.html)

#pragma once

#include "Player.h"
#include "PostProcessController.h"
#include "Errors.h"
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Navigation/NavigationMesh.h>
#include <CleanupNs.h>

class FwClient;

class BaseLevel : public Object
{
    URHO3D_OBJECT(BaseLevel, Object)
public:
    BaseLevel(Context* context) :
        Object(context),
        uiRoot_(GetSubsystem<UI>()->GetRoot()),
        scene_(nullptr),
        cameraNode_(nullptr),
        player_(nullptr),
        yaw_(0.0f),
        pitch_(0.0f),
        debugGeometry_(false)
    {}

    virtual ~BaseLevel() override
    {
        UnsubscribeFromAllEvents();
        Dispose();
    }

    virtual void Run();
    virtual void Pause();
protected:
    friend class FwClient;
    virtual void CreateUI();
    virtual void CreateScene();
    void LoadScene(const String& file);
    virtual void Dispose();
    virtual void SubscribeToEvents();
    virtual void SetupViewport();
    void CreateLogo();
    virtual void Update(StringHash, VariantMap&) {}
    virtual void PostUpdate(StringHash, VariantMap&) {}
    virtual void PostRenderUpdate(StringHash eventType, VariantMap& eventData);

    virtual void OnNetworkError(Client::ConnectionError connectionError, const std::error_code& err);
    virtual void OnProtocolError(AB::ErrorCodes err);

    Ray GetActiveViewportScreenRay(const IntVector2& pos) const
    {
        if (viewport_)
            return viewport_->GetScreenRay(pos.x_, pos.y_);
        return Ray();
    }
    void InitModelAnimations();

    Urho3D::UIElement* uiRoot_;
    SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
    SharedPtr<Player> player_;
    SharedPtr<Viewport> viewport_;
    SharedPtr<PostProcessController> postProcess_;
    /// Camera yaw angle.
    float yaw_;
    /// Camera pitch angle.
    float pitch_;
    SharedPtr<Sprite> logoSprite_;
public:
    bool debugGeometry_;
    /// If this level has a player call this to create it
    void ToggleDebugGeometry()
    {
        debugGeometry_ = !debugGeometry_;
    }
    virtual void ShowError(const String& message, const String& title = "Error");
    Player* GetPlayer() const { return player_.Get(); }
    Camera* GetCamera() const
    {
        if (cameraNode_)
            return cameraNode_->GetComponent<Camera>();
        return nullptr;
    }
    PostProcessController* GetPostProcessController() const
    {
        if (postProcess_)
            return postProcess_.Get();
        return nullptr;
    }
private:
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
};
