#pragma once

class ClientPrediction : public LogicComponent
{
    URHO3D_OBJECT(ClientPrediction, LogicComponent);
private:
    int64_t serverTime_;
    Vector3 serverPos_;
    uint8_t lastMoveDir_ = 0;
    uint8_t lastTurnDir_ = 0;
    void UpdateMove(float timeStep, uint8_t direction, float speedFactor);
    void Move(float speed, const Vector3& amount);
    void UpdateTurn(float timeStep, uint8_t direction, float speedFactor);
    void Turn(float yAngle);
    void TurnAbsolute(float yAngle);
    inline float GetSpeed(float timeElapsed, float baseSpeed, float speedFactor)
    {
        return ((timeElapsed * 1000.0f) / baseSpeed) * speedFactor;
    }
    bool CheckCollision(const Vector3& pos);
public:
    static void RegisterObject(Context* context);

    ClientPrediction(Context* context);
    ~ClientPrediction();

    /// Called on scene update, variable timestep.
    void FixedUpdate(float timeStep) override;
    void CheckServerPosition(int64_t time, const Vector3& serverPos);
    void CheckServerRotation(int64_t time, float rad);
};

