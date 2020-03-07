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

#include <stdint.h>
#include <sa/Iteration.h>
#include <sa/Noncopyable.h>

namespace Math {
class BoundingBox;
class Vector3;
}

namespace Game {

class Actor;
class GameObject;

namespace Components {

/// Only an Actor can have a CollisionComp, because only moving objects need it.
class CollisionComp
{
    NON_COPYABLE(CollisionComp)
private:
    Actor& owner_;
    bool isCollidingWithPlayers_{ true };
    Iteration CollisionCallback(const Math::BoundingBox& myBB, GameObject& other, const Math::Vector3& move, bool& updateTrans);
    bool Slide(const Math::BoundingBox& myBB, const GameObject& other);
    void GotoSafePosition();
    static Math::Vector3 GetBodyCenter(const Math::Vector3& pos);
public:
    CollisionComp() = delete;
    explicit CollisionComp(Actor& owner) :
        owner_(owner)
    { }
    ~CollisionComp() = default;

    void Update(uint32_t timeElapsed);
    void ResolveCollisions();
};

}
}
