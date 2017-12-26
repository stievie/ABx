#pragma once

class PingDot : public Button
{
    URHO3D_OBJECT(PingDot, Button);
private:
    float lastUpdate_;
    SharedPtr<Text> tooltipText_;
public:
    static void RegisterObject(Context* context);

    PingDot(Context* context);
    ~PingDot();

    void Update(float timeStep) override;
};

