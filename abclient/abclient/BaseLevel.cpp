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

#include "stdafx.h"
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
    URHO3D_LOGERRORF("Network error (%d): %s", err.default_error_condition().value(), err.message().c_str());
    String msg;
    switch (connectionError)
    {
    case Client::ConnectionError::ResolveError:
        msg = "Resolve error: ";
        break;
    case Client::ConnectionError::WriteError:
        msg = "Write error: ";
        break;
    case Client::ConnectionError::ConnectError:
        msg = "Connect error: ";
        break;
    case Client::ConnectionError::ReceiveError:
        msg = "Read error: ";
        break;
    case Client::ConnectionError::ConnectTimeout:
        msg = "Connect timeout: ";
        break;
    case Client::ConnectionError::ReadTimeout:
        msg = "Read timeout: ";
        break;
    case Client::ConnectionError::WriteTimeout:
        msg = "Write timeout: ";
        break;
    default:
        msg = "Network Error: ";
        break;
    }
    msg += String(err.message().c_str());
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

void BaseLevel::InitSunProperties()
{
    // https://discourse.urho3d.io/t/better-shadows-possible-three-issues/1013/3
    // https://discourse.urho3d.io/t/shadow-on-slopes/4629
    Node* sunNode = scene_->GetChild("Sun", false);
    if (sunNode)
    {
        Light* sun = sunNode->GetComponent<Light>();
        if (sun)
        {
            sun->SetBrightness(1.0f);
            sun->SetShadowFadeDistance(100.0f);
            sun->SetShadowDistance(125.0f);

            sun->SetShadowBias(BiasParameters(0.0000025f, 1.0f));
            sun->SetShadowCascade(CascadeParameters(20.0f, 60.0f, 180.0f, 560.0f, 0.1f, 0.1f));
            //sun->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
            sun->SetShadowResolution(1.0);
            sun->SetCastShadows(true);
        }
    }
    else
        URHO3D_LOGWARNING("No Sun node found");
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
