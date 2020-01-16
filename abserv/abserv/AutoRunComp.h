#pragma once

#include <memory>
#include "Vector3.h"
#include "Quaternion.h"
#include "Mechanic.h"
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

