#pragma once

class EffectsWindow : public UIElement
{
    URHO3D_OBJECT(EffectsWindow, UIElement);
private:
    unsigned effectCount_;
public:
    static void RegisterObject(Context* context);

    EffectsWindow(Context* context);
    ~EffectsWindow();

    void EffectAdded(uint32_t effectIndex, uint32_t ticks);
    void EffectRemoved(uint32_t effectIndex);
};

