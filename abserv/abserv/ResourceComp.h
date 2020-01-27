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

#include <cassert>
#include "NetworkMessage.h"
#include "MathUtils.h"
#include <sa/Noncopyable.h>

namespace Game {

class Actor;
class Skill;

namespace Components {

// Lua definition in /scripts/includes/consts.lua
enum class SetValueType
{
    Absolute,
    Increase,
    IncreasePercent,
    Decrease,
    DecreasePercent,
};

enum ResourceDirty
{
    DirtyEnergy = 1,
    DirtyHealth = 1 << 1,
    DirtyAdrenaline = 1 << 2,
    DirtyOvercast = 1 << 3,
    DirtyHealthRegen = 1 << 4,
    DirtyEnergyRegen = 1 << 5,
    DirtyMaxHealth = 1 << 6,
    DirtyMaxEnergy = 1 << 7,
    DirtyMorale = 1 << 8,
};

// Lua definition in /scripts/includes/consts.lua
enum class ResourceType
{
    None = 0,
    Energy,
    Health,
    Adrenaline,
    Overcast,
    HealthRegen,
    EnergyRegen,
    MaxHealth,
    MaxEnergy
};

class ResourceComp
{
    NON_COPYABLE(ResourceComp)
private:
    Actor& owner_;
    float energy_{ 0.0f };
    float health_{ 0.0f };
    float adrenaline_{ 0.0f };
    float overcast_{ 0.0f };
    float healthRegen_{ 0.0f };     // How many arrows right or left
    float energyRegen_{ 2.0f };
    int naturalHealthRegen_{ 0 };
    int maxHealth_{ 0 };
    int maxEnergy_{ 0 };
    int morale_{ 0 };
    uint32_t dirtyFlags_{ 0 };
    int64_t lastHpDecrease_{ 0 };
    int64_t lastRegenIncrease_{ 0 };
    template <typename T>
    static bool SetValue(SetValueType t, T value, T maxVal, T& out)
    {
        switch (t)
        {
        case SetValueType::Absolute:
            if (!Math::Equals(out, value))
            {
                out = value;
                return true;
            }
            break;
        case SetValueType::Decrease:
            // Must be positive value
            if (value < static_cast<T>(0))
                return SetValue(SetValueType::Increase, -value, maxVal, out);
            if (!Math::Equals(value, static_cast<T>(0)))
            {
                T oldVal = out;
                out -= value;
                return !Math::Equals(oldVal, out);
            }
            break;
        case SetValueType::DecreasePercent:
            // Percent of current value
            return SetValue(SetValueType::Decrease, static_cast<T>((static_cast<float>(out) / 100.0f) * static_cast<float>(value)), maxVal, out);
        case SetValueType::Increase:
            // Must be positive value
            if (value < static_cast<T>(0))
                return SetValue(SetValueType::Decrease, -value, maxVal, out);
            if (!Math::Equals(value, static_cast<T>(0)))
            {
                if (!Math::Equals(maxVal, static_cast<T>(0)) && Math::Equals(out, maxVal))
                    return false;
                T oldVal = out;
                out += value;
                out = std::min(out, maxVal);
                return !Math::Equals(oldVal, out);
            }
            break;
        case SetValueType::IncreasePercent:
            // Percent of current value
            return SetValue(SetValueType::Increase, static_cast<T>((static_cast<float>(out) / 100.0f) * static_cast<float>(value)), maxVal, out);
        }
        return false;
    }
    void UpdateRegen(uint32_t timeElapsed);
    uint32_t GetLastHpDecrease() const;
public:
    ResourceComp() = delete;
    explicit ResourceComp(Actor& owner);
    ~ResourceComp() = default;

    void UpdateResources();
    int GetBaseHealth() const;

    int GetMorale() const { return morale_; }
    bool IncreaseMorale();
    bool DecreaseMorale();
    int GetHealth() const { return static_cast<int>(health_); }
    void SetHealth(SetValueType t, int value);
    /// Get health / max health. Value between 0..1
    float GetHealthRatio() const {
        assert(maxHealth_ > 0);
        return Math::Clamp(health_, 0.0f, static_cast<float>(maxHealth_)) / static_cast<float>(maxHealth_);
    }
    int GetEnergy() const { return static_cast<int>(energy_); }
    void SetEnergy(SetValueType t, int value);
    float GetEnergyRatio() const {
        assert(maxEnergy_ > 0);
        return Math::Clamp(energy_, 0.0f, static_cast<float>(maxEnergy_)) / static_cast<float>(maxEnergy_);
    }
    int GetAdrenaline() const { return static_cast<int>(adrenaline_); }
    void SetAdrenaline(SetValueType t, int value);
    int GetOvercast() const { return static_cast<int>(overcast_); }
    void SetOvercast(SetValueType t, int value);
    int GetHealthRegen() const { return static_cast<int>(healthRegen_) + naturalHealthRegen_; }
    void SetHealthRegen(SetValueType t, int value);
    int GetEnergyRegen() const { return static_cast<int>(energyRegen_); }
    void SetEnergyRegen(SetValueType t, int value);
    int GetMaxHealth() const { return maxHealth_; }
    void SetMaxHealth(int value);
    int GetMaxEnergy() const { return maxEnergy_; }
    void SetMaxEnergy(int value);
    int GetValue(ResourceType type) const;
    void SetValue(ResourceType type, SetValueType t, int value);
    /// Steal energy from this actor. The source must add the returned value to its energy.
    int DrainEnergy(int value);
    int AddEnergy(int value);
    /// Approx. check if we have enough resource to use this skill.
    bool HaveEnoughResources(const Skill* skill) const;

    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message, bool ignoreDirty = false);
};

}
}

