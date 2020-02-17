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

#include <memory>
#include <absmath/Vector3.h>
#include <absmath/Quaternion.h>
#include <abshared/Mechanic.h>
#include <sa/Noncopyable.h>

namespace Game {

class Actor;
class GameObject;

namespace Components {

class AutoRunComp
{
    NON_COPYABLE(AutoRunComp)
private:
    static constexpr uint32_t RECALCULATE_PATH_TIME = 1000;
    Actor& owner_;
    int64_t lastCalc_{ 0 };
    /// Maximum distance to consider being there
    float maxDist_{ RANGE_TOUCH };
    bool autoRun_{ false };
    std::vector<Math::Vector3> wayPoints_;
    Math::Vector3 destination_;
    std::weak_ptr<Actor> following_;
    // Remove the first way points
    void Pop();
    // Get next waypoint
    Math::Vector3 Next();
    void MoveTo(uint32_t timeElapsed, const Math::Vector3& dest);
    bool FindPath(const Math::Vector3& dest);
    // Stop auto running and set state to idle
    void StopAutoRun();
    void OnCollide(GameObject* other);
    void OnStuck();
    Math::Vector3 AvoidObstaclesInternal(const Math::Vector3& destination, unsigned recursionLevel);
    Math::Vector3 AvoidObstacles(const Math::Vector3& destination);
public:
    AutoRunComp() = delete;
    explicit AutoRunComp(Actor& owner);
    ~AutoRunComp() = default;

    bool Follow(std::shared_ptr<GameObject> object, bool ping, float maxDist = RANGE_TOUCH);
    bool Goto(const Math::Vector3& dest);
    bool GotoDirection(const Math::Quaternion& direction, float distance);
    void Reset();
    bool HasWaypoints() const
    {
        return wayPoints_.size() != 0;
    }
    void Update(uint32_t timeElapsed);
    void SetAutoRun(bool value);
    bool IsAutoRun() const { return autoRun_; }
    bool IsFollowing(const Actor& actor) const;
};

}
}

