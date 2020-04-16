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
#include <vector>
#include <memory>
#include <abshared/Damage.h>
#include <sa/CircularQueue.h>
#include <sa/Noncopyable.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class Actor;
class Skill;

namespace Components {

class DamageComp
{
    NON_COPYABLE(DamageComp)
    NON_MOVEABLE(DamageComp)
private:
    // Keep some history
    static constexpr uint32_t DAMAGEHISTORY_TOKEEP = 25;
    // The AI may check if we got some damage. This is the time how long to go back in history.
    static constexpr uint32_t LAST_DAMAGE_TIME = 5000;
    struct DamageItem
    {
        struct
        {
            int64_t tick;
            DamageType type;
            DamagePos pos;
            int value;
            uint32_t actorId;
            // The skill/effect causing this damage. If 0 it's a melee damage.
            uint32_t index;
        } damage;
        bool dirty;
    };

    Actor& owner_;
    // Damage history kept for DAMAGEHISTORY_TOKEEP
    sa::CircularQueue<DamageItem, DAMAGEHISTORY_TOKEEP> damages_;
    std::weak_ptr<Actor> lastDamager_;
    std::weak_ptr<Actor> lastMeleeDamager_;
public:
    DamageComp() = delete;
    explicit DamageComp(Actor& owner);
    ~DamageComp() = default;

    void Update(uint32_t /* timeElapsed */) { }
    void ApplyDamage(Actor* source, uint32_t index, DamageType type, int value, float penetration, bool melee);
    /// Steal life from this actor. The source must add the returned value to its life.
    int DrainLife(Actor* source, uint32_t index, int value);
    void Touch();
    DamagePos GetDamagePos() const;
    // Return true if we are currently getting melee damage
    bool IsGettingMeleeDamage() const;
    /// How long the actor didn't get damage in ms
    uint32_t NoDamageTime() const;
    std::shared_ptr<Actor> GetLastDamager() const { return lastDamager_.lock(); }
    std::shared_ptr<Actor> GetLastMeleeDamager() const { return lastMeleeDamager_.lock(); }
    bool GotDamageType(DamageType type) const;
    bool GotDamageCategory(DamageTypeCategory cat) const;

    bool IsLastDamager(const Actor& actor);
    void Write(Net::NetworkMessage& message);

    int64_t lastDamage_{ 0 };
    int64_t lastMeleeDamage_{ 0 };
};

}
}

