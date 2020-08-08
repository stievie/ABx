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


#include "BaseLevel.h"
#include "LevelManager.h"
#include <Urho3D/UI/MessageBox.h>
#include "FwClient.h"
#include "ClientApp.h"

//#include <Urho3D/DebugNew.h>

void BaseLevel::Run()
{
    if (scene_)
    {
        scene_->SetUpdateEnabled(true);
    }
}

void BaseLevel::Pause()
{
    if (scene_)
    {
        scene_->SetUpdateEnabled(false);
    }
}

void BaseLevel::Dispose()
{
    // Pause the scene, remove all contents from the scene, then remove the scene itself.
    if (scene_)
    {
        scene_->SetUpdateEnabled(false);
        scene_->Clear();
        scene_->Remove();
        scene_ = nullptr;
    }
}

void BaseLevel::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(BaseLevel, HandleUpdate));
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(BaseLevel, HandlePostUpdate));
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(BaseLevel, HandlePostRenderUpdate));
}

void BaseLevel::ShowError(const String& message, const String& title)
{
    using MsgBox = Urho3D::MessageBox;
    /* MsgBox* msgBox = */ new MsgBox(context_, message, title);
}

void BaseLevel::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    Update(eventType, eventData);
}

void BaseLevel::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    PostUpdate(eventType, eventData);
}

void BaseLevel::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    PostRenderUpdate(eventType, eventData);
}

void BaseLevel::PostRenderUpdate(StringHash, VariantMap&)
{
    if (debugGeometry_)
    {
        if (auto physW = scene_->GetComponent<PhysicsWorld>())
            physW->DrawDebugGeometry(true);
        if (auto navMesh = scene_->GetComponent<NavigationMesh>())
            navMesh->DrawDebugGeometry(true);
    }
}

void BaseLevel::OnNetworkError(Client::ConnectionError connectionError, const std::error_code& err)
{
    String msg = String(Client::Client::GetNetworkErrorMessage(connectionError));
    URHO3D_LOGERRORF("Network error [%s]: (%d) %s", msg.CString(), err.default_error_condition().value(), err.message().c_str());
    msg += ": " + String(err.message().c_str());
    ShowError(msg, "Network Error");
}

void BaseLevel::OnProtocolError(AB::ErrorCodes err)
{
    String msg = FwClient::GetProtocolErrorMessage(err);
    URHO3D_LOGERRORF("Protocol error (%d): %s", err, msg.CString());

    if (err == AB::ErrorCodes::TokenAuthFailure)
    {
        // Expired/invalid token -> re-login
        VariantMap& e = GetEventDataMap();
        using namespace Events::SetLevel;
        e[P_NAME] = "LoginLevel";
        SendEvent(Events::E_SETLEVEL, e);
        return;
    }

    if (!msg.Empty())
        ShowError(msg, "Error");
}

void BaseLevel::InitModelAnimations()
{
    PODVector<Node*> nodes;
    if (scene_->GetNodesWithTag(nodes, "HasAnimation"))
    {
        for (const auto node : nodes)
        {
            const String& fileName = node->GetVar("AnimationFile").GetString();
            AnimationController* animCtrl = node->GetComponent<AnimationController>();
            if (animCtrl)
            {
                animCtrl->PlayExclusive(fileName, 0, true);
            }
        }
    }
}

void BaseLevel::SetupViewport()
{
    Renderer* renderer = GetSubsystem<Renderer>();

    viewport_ = new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>());
    renderer->SetViewport(0, viewport_);

    postProcess_ = scene_->CreateComponent<PostProcessController>();
    postProcess_->AddViewport(viewport_, true);
    Options* options = GetSubsystem<Options>();
    postProcess_->SetUseFXAA3(options->GetAntiAliasingMode() == AntiAliasingMode::FXAA3);
}

void BaseLevel::CreateLogo()
{
    // Get logo texture
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Texture2D* logoTexture = cache->GetResource<Texture2D>("Textures/Trill.png");
    if (!logoTexture)
        return;

    // Create logo sprite and add to the UI layout
    UI* ui = GetSubsystem<UI>();
    logoSprite_ = ui->GetRoot()->CreateChild<Sprite>();

    // Set logo sprite texture
    logoSprite_->SetTexture(logoTexture);

    int textureWidth = logoTexture->GetWidth();
    int textureHeight = logoTexture->GetHeight();

    // Set logo sprite scale
    logoSprite_->SetScale(100.0f / textureWidth);

    // Set logo sprite size
    logoSprite_->SetSize(textureWidth, textureHeight);

    // Set logo sprite hot spot
    logoSprite_->SetHotSpot(textureWidth, textureHeight);

    // Set logo sprite alignment
    logoSprite_->SetAlignment(HA_RIGHT, VA_BOTTOM);

    // Make logo not fully opaque to show the scene underneath
    logoSprite_->SetOpacity(0.7f);

    // Set a low priority for the logo so that other UI elements can be drawn on top
    logoSprite_->SetPriority(-100);
}

void BaseLevel::CreateUI()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* style = cache->GetResource<XMLFile>("UI/FwDefaultStyle.xml");
    uiRoot_->SetDefaultStyle(style);
}

void BaseLevel::CreateScene()
{
    scene_ = new Scene(context_);
}
