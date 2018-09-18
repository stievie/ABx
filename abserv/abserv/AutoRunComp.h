#pragma once

#include "Vector3.h"

namespace Game {

class Actor;
class GameObject;

namespace Components {

class AutoRunComp
{
private:
    Actor& owner_;
    int64_t lastCalc_;
    /// Maximum distance to consider being there
    float maxDist_;
    std::vector<Math::Vector3> wayPoints_;
    Math::Vector3 destination_;
    std::weak_ptr<Actor> following_;
    void Pop();
    const Math::Vector3& Next() const
    {
        return wayPoints_[0];
    }
    void MoveTo(uint32_t timeElapsed, const Math::Vector3& dest);
    bool FindPath(const Math::Vector3& dest);
public:
    AutoRunComp(Actor& owner) :
        owner_(owner),
        lastCalc_(0),
        maxDist_(1.0f),
        autoRun_(false)
    { }
    ~AutoRunComp() = default;

    bool Follow(std::shared_ptr<GameObject> object);
    bool Goto(const Math::Vector3& dest);
    void Reset()
    {
        wayPoints_.clear();
        autoRun_ = false;
        following_.reset();
    }
    bool HasWaypoints() const
    {
        return wayPoints_.size() != 0;
    }
    void Update(uint32_t timeElapsed);

    bool autoRun_;
};

}
}

