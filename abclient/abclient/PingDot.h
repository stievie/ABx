#pragma once

#include <Urho3DAll.h>

class PingDot : public Button
{
    URHO3D_OBJECT(PingDot, Button)
private:
    float lastUpdate_;
    SharedPtr<Text> tooltipText_;
    static const IntRect PING_NONE;
    static const IntRect PING_GOOD;
    static const IntRect PING_OKAY;
    static const IntRect PING_BAD;
public:
    static void RegisterObject(Context* context);

    PingDot(Context* context);
    ~PingDot() override;

    void Update(float timeStep) override;
};

