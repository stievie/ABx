#pragma once

#include "Vector3.h"

namespace Game {

class Creature;

namespace Components {

class AutoRunComp
{
private:
    Creature& owner_;
    std::vector<Math::Vector3> wayPoints_;
    Math::Vector3 destination_;
    void Pop();
    const Math::Vector3& Next() const
    {
        return wayPoints_[0];
    }
    void MoveTo(uint32_t timeElapsed, const Math::Vector3& dest);
public:
    AutoRunComp(Creature& owner) :
        owner_(owner),
        autoRun_(false)
    { }
    ~AutoRunComp() = default;

    bool FindPath(const Math::Vector3& dest);
    void Reset()
    {
        wayPoints_.clear();
        autoRun_ = false;
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

