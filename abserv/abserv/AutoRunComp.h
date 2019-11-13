#pragma once

#include <memory>
#include "Vector3.h"
#include "Quaternion.h"
#include "Mechanic.h"

namespace Game {

class Actor;
class GameObject;

namespace Components {

class AutoRunComp
{
private:
    Actor& owner_;
    int64_t lastCalc_{ 0 };
    /// Maximum distance to consider being there
    float maxDist_{ 1.0f };
    bool autoRun_{ false };
    std::vector<Math::Vector3> wayPoints_;
    Math::Vector3 destination_;
    std::weak_ptr<Actor> following_;
    void Pop();
    Math::Vector3 Next();
    void MoveTo(uint32_t timeElapsed, const Math::Vector3& dest);
    bool FindPath(const Math::Vector3& dest);
    void StopAutoRun();
    void OnCollide(GameObject* other);
    void OnStuck();
    Math::Vector3 AvoidObstacles(const Math::Vector3& destination);
public:
    static constexpr float SWITCH_WAYPOINT_DIST = 2.0f;
    AutoRunComp() = delete;
    explicit AutoRunComp(Actor& owner);
    // non-copyable
    AutoRunComp(const AutoRunComp&) = delete;
    AutoRunComp& operator=(const AutoRunComp&) = delete;
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
};

}
}

