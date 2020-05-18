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

#include <absmath/Vector3.h>
#include <sa/Noncopyable.h>
#include <eastl.hpp>

namespace Game {

class Npc;

namespace Components {

class WanderComp
{
    NON_COPYABLE(WanderComp)
    NON_MOVEABLE(WanderComp)
private:
    enum class Direction
    {
        Forward,
        Backward
    };
    Npc& owner_;

    ea::vector<Math::Vector3> route_;
    Direction direction_{ Direction::Forward };
    int currentIndex_{ -1 };
    bool wandering_{ false };
    bool initialized_{ false };
    int FindCurrentPointIndex() const;
    int GetNextIndex();
    bool GotoCurrentPoint();
    bool CheckDestination() const;
    void Initialize();
public:
    WanderComp() = delete;
    explicit WanderComp(Npc& owner);
    ~WanderComp() = default;

    void AddRoutePoint(const Math::Vector3& point);
    bool HaveRoute() const { return route_.size() != 0; }
    void Update(uint32_t timeElapsed);
    bool IsWandering() const { return wandering_; }
    bool Wander(bool value);
};

}
}
