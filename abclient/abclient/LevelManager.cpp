#include "stdafx.h"
#include "LevelManager.h"
#include "BaseLevel.h"

LevelManager::LevelManager(Context* context) :
    Object(context),
    fadeWindow_(nullptr),
    levelName_(""),
    lastLevelName_(""),
    level_(nullptr),
    drawDebugGeometry_(false),
    fadeStatus_(0)
{
    // Listen to set level event
    SubscribeToEvent(AbEvents::E_SET_LEVEL, URHO3D_HANDLER(LevelManager, HandleSetLevelQueue));
}

LevelManager::~LevelManager()
{
    level_ = SharedPtr<Object>();
}

void LevelManager::HandleSetLevelQueue(StringHash eventType, VariantMap& eventData)
{
    UNREFERENCED_PARAMETER(eventType);

    // Busy now
    if (levelQueue_.Size())
    {
        return;
    }
    // Push to queue
    levelQueue_.Push(eventData);

    // Subscribe HandleUpdate() function for processing update events
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(LevelManager, HandleUpdate));

    // Init fade status
    fadeStatus_ = 0;
}

void LevelManager::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    UNREFERENCED_PARAMETER(eventType);

    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    // Move sprites, scale movement with time step
    fadeTime_ -= timeStep;

    switch (fadeStatus_)
    {
    case 0:
    {
        // Prepare to fade out
        if (!level_)
        {
            // No old level
            fadeStatus_++;
            break;
        }
        // Add a fade layer
        AddFadeLayer();
        fadeWindow_->SetOpacity(0.0f);
        fadeTime_ = MAX_FADE_TIME;
        fadeStatus_++;
        break;
    }
    case 1:
    {
        // Fade out
        if (!level_)
        {
            // No old level
            fadeStatus_++;
            break;
        }
        fadeWindow_->SetOpacity(1.0f - fadeTime_ / MAX_FADE_TIME);

        // Increase fade status
        if (fadeTime_ <= 0.0f)
        {
            fadeStatus_++;
        }
        break;
    }
    case 2:
    {
        // Release old level
        if (!level_)
        {
            // No old level
            fadeStatus_++;
            break;
        }
        // We can not create new level here, or it may cause errors, we have to create it at the next update point.
        level_ = SharedPtr<Object>();
        fadeStatus_++;
        break;
    }
    case 3:
    {
        // Create new level
        lastLevelName_ = levelName_;
        VariantMap& levelData = levelQueue_.Front();
        levelName_ = levelData[AbEvents::E_SET_LEVEL].GetString();
        mapName_ = levelData[AbEvents::ED_MAP_NAME].GetString();
        level_ = context_->CreateObject(StringHash(levelName_));
        BaseLevel* baseLevel = dynamic_cast<BaseLevel*>(GetCurrentLevel());
        if (baseLevel)
        {
            baseLevel->debugGeometry_ = drawDebugGeometry_;
        }
        // Add a fade layer
        if (!fadeWindow_)
            AddFadeLayer();
        fadeWindow_->SetOpacity(1.0f);
        fadeTime_ = MAX_FADE_TIME;
        fadeStatus_++;
        break;
    }
    case 4:
    {
        // Fade in
        fadeWindow_->SetOpacity(fadeTime_ / MAX_FADE_TIME);

        // Increase fade status
        if (fadeTime_ <= 0.0f)
        {
            fadeStatus_++;
        }
        break;
    }
    case 5:
    {
        // Finished
        // Remove fade layer
        fadeWindow_->Remove();
        fadeWindow_ = SharedPtr<Window>();

        // Unsubscribe update event
        UnsubscribeFromEvent(E_UPDATE);
        // Remove the task
        levelQueue_.PopFront();
        // Release all unused resources
        GetSubsystem<ResourceCache>()->ReleaseAllResources(false);
        break;
    }
    }
}

void LevelManager::AddFadeLayer()
{
    fadeWindow_ = new Window(context_);
    // Make the window a child of the root element, which fills the whole screen.
    GetSubsystem<UI>()->GetRoot()->AddChild(fadeWindow_);
    fadeWindow_->SetSize(GetSubsystem<Graphics>()->GetWidth(), GetSubsystem<Graphics>()->GetHeight());
    fadeWindow_->SetLayout(LM_FREE);
    // Urho has three layouts: LM_FREE, LM_HORIZONTAL and LM_VERTICAL.
    // In LM_FREE the child elements of this window can be arranged freely.
    // In the other two they are arranged as a horizontal or vertical list.

    // Center this window in it's parent element.
    fadeWindow_->SetAlignment(HA_CENTER, VA_CENTER);
    // Black color
    fadeWindow_->SetColor(Color(0.0f, 0.0f, 0.0f, 1.0f));
    // Make it top most
    fadeWindow_->SetBringToBack(false);
    fadeWindow_->BringToFront();
}

void LevelManager::SetDrawDebugGeometry(bool draw)
{
    if (drawDebugGeometry_ != draw)
    {
        drawDebugGeometry_ = draw;
        BaseLevel* baseLevel = dynamic_cast<BaseLevel*>(GetCurrentLevel());
        if (baseLevel)
            baseLevel->debugGeometry_ = drawDebugGeometry_;
    }
}
