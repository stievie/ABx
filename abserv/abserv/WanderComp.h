#pragma once

#include "Vector3.h"

namespace Game {

class Npc;

namespace Components {

class WanderComp
{
private:
    enum class Direction
    {
        Forward,
        Backward
    };
    Npc& owner_;

    std::vector<Math::Vector3> route_;
    Direction direction_{ Direction::Forward };
    int currentIndex_{ -1 };
    bool wandering_{ false };
    int FindCurrentPointIndex() const;
    int GetNextIndex();
    bool GotoCurrentPoint();
    bool CheckDestination() const;
public:
    WanderComp() = delete;
    explicit WanderComp(Npc& owner);
    // non-copyable
    WanderComp(const WanderComp&) = delete;
    WanderComp& operator=(const WanderComp&) = delete;
    ~WanderComp() = default;

    void AddRoutePoint(const Math::Vector3& point);
    bool HaveRoute() const { return route_.size() != 0; }
    void Update(uint32_t timeElapsed);
    bool IsWandering() const { return wandering_; }
    bool Wander(bool value);
};

}
}
