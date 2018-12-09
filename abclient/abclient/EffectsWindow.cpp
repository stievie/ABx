#include "stdafx.h"
#include "EffectsWindow.h"
#include "FwClient.h"

void EffectsWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<EffectsWindow>();
}

EffectsWindow::EffectsWindow(Context* context) :
    UIElement(context),
    effectCount_(0)
{
    SetName("EffectsWindow");
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    XMLFile *file = cache->GetResource<XMLFile>("UI/EffectsWindow.xml");
    LoadChildXML(file->GetRoot(), nullptr);

    SetAlignment(HA_CENTER, VA_BOTTOM);
    SetPosition(0, -100);
    // It seems this isn't loaded from the XML file
    SetLayoutMode(LM_HORIZONTAL);
    SetLayoutBorder(IntRect(4, 4, 4, 4));
    SetPivot(0, 0);
    SetOpacity(0.9f);
    SetMinSize(50, 50);
    SetVisible(false);
}

EffectsWindow::~EffectsWindow()
{
    UnsubscribeFromAllEvents();
}

void EffectsWindow::EffectAdded(uint32_t effectIndex, uint32_t ticks)
{
    FwClient* client = GetSubsystem<FwClient>();
    const AB::Entities::Effect* effect = client->GetEffectByIndex(effectIndex);
    if (!effect)
        return;

    String name = "Effect_" + String(effectIndex);
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    ++effectCount_;

    BorderImage* effectIcon = dynamic_cast<BorderImage*>(GetChild(name, true));
    if (!effectIcon)
        effectIcon = CreateChild<BorderImage>(name);
    Texture2D* icon = cache->GetResource<Texture2D>(String(effect->icon.c_str()));
    effectIcon->SetVar("Index", effectIndex);
    effectIcon->SetVar("Ticks", ticks);
    effectIcon->SetSize(50, 50);
    effectIcon->SetMinSize(50, 50);
    effectIcon->SetOpacity(1.0f);
    effectIcon->SetTexture(icon);
    effectIcon->SetImageRect(IntRect(0, 0, 256, 256));
    effectIcon->SetBorder(IntRect(4, 4, 4, 4));
//    effectIcon->SetPosition((effectCount_ - 1) * 50, 0);
    SetWidth(50 * effectCount_);
//    SetHeight(50);
    SetVisible(effectCount_ != 0);
    UpdateLayout();
}

void EffectsWindow::EffectRemoved(uint32_t effectIndex)
{
    String name = "Effect_" + String(effectIndex);
    BorderImage* effectIcon = dynamic_cast<BorderImage*>(GetChild(name, true));
    if (effectIcon)
    {
        --effectCount_;
        RemoveChild(effectIcon);
        SetWidth(50 * effectCount_);
        SetVisible(effectCount_ != 0);
        UpdateLayout();
    }
}

void EffectsWindow::Clear()
{
    SetVisible(false);
    RemoveAllChildren();
    effectCount_ = 0;
    SetWidth(0);
}
