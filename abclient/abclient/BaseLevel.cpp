#include "stdafx.h"
#include "BaseLevel.h"
#include "AbEvents.h"
#include "LevelManager.h"
#include <Urho3D/UI/MessageBox.h>
#include "FwClient.h"

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
    MsgBox* msgBox = new MsgBox(context_, message, title);
}

void BaseLevel::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    Update(eventType, eventData);
}

void BaseLevel::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    PostUpdate(eventType, eventData);
}

void BaseLevel::HandlePostRenderUpdate(StringHash eventType, VariantMap & eventData)
{
    PostRenderUpdate(eventType, eventData);
}

void BaseLevel::PostRenderUpdate(StringHash eventType, VariantMap & eventData)
{
    UNREFERENCED_PARAMETER(eventType);
    UNREFERENCED_PARAMETER(eventData);

//    if (debugGeometry_)
//        scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
}

void BaseLevel::OnNetworkError(const std::error_code& err)
{
    ShowError(String(err.message().c_str()), "Network Error");
}

void BaseLevel::OnProtocolError(uint8_t err)
{
    String msg = FwClient::GetProtocolErrorMessage(err);
    if (!msg.Empty())
        ShowError(msg, "Error");
}

void BaseLevel::SetupViewport()
{
    Graphics* graphics = GetSubsystem<Graphics>();
    Renderer* renderer = GetSubsystem<Renderer>();

    viewport_ = new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>());
    renderer->SetViewport(0, viewport_);

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SharedPtr<RenderPath> effectRenderPath = viewport_->GetRenderPath()->Clone();
    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA2.xml"));
    effectRenderPath->SetEnabled("FXAA2", false);
    viewport_->SetRenderPath(effectRenderPath);
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
