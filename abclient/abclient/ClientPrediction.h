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

#pragma once

#include <Urho3DAll.h>
#include <AB/ProtocolCodes.h>

class HeightMap;

class ClientPrediction : public LogicComponent
{
    URHO3D_OBJECT(ClientPrediction, LogicComponent)
private:
    int64_t serverTime_;
    int64_t lastStateChange_;
    Vector3 serverPos_;
    uint8_t lastMoveDir_ = 0;
    uint8_t lastTurnDir_ = 0;
    mutable SharedPtr<Terrain> terrain_;
    mutable SharedPtr<HeightMap> heightMap_;
    AB::GameProtocol::CreatureState lastState_{ AB::GameProtocol::CreatureState::Idle };
    void UpdateMove(float timeStep, uint8_t direction, float speedFactor);
    void Move(float speed, const Vector3& amount);
    void UpdateTurn(float timeStep, uint8_t direction, float speedFactor);
    void Turn(float yAngle);
    void TurnAbsolute(float yAngle);
    float GetSpeed(float timeElapsed, float baseSpeed, float speedFactor)
    {
        return ((timeElapsed * 1000.0f) / baseSpeed) * speedFactor;
    }
    bool CheckCollision(const Vector3& pos);
    float GetHeight(const Vector3& world) const;
    Terrain* GetTerrain() const;
    HeightMap* GetHeightMap() const;
public:
    static void RegisterObject(Context* context);

    ClientPrediction(Context* context);
    ~ClientPrediction() override;

    /// Called on scene update, variable timestep.
    void FixedUpdate(float timeStep) override;
    void CheckServerPosition(int64_t time, const Vector3& serverPos);
    void CheckServerRotation(int64_t time, float rad);
};

