#pragma once

#include "NetworkMessage.h"
#include "MathUtils.h"

namespace Game {

class Actor;

namespace Components {

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
    DirtyMaxEnergy = 1 << 7
};

static constexpr float MAX_HEALTH_REGEN = 10.0f;
static constexpr float MAX_ENERGY_REGEN = 10.0f;

class ResourceComp
{
private:
    Actor& owner_;
    float energy_;
    float health_;
    float adrenaline_;
    float overcast_;
    float healthRegen_;     // How many arrows right or left
    float energyRegen_;
    int maxHealth_;
    int maxEnergy_;
    uint32_t dirtyFlags_;
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
            return SetValue(SetValueType::Decrease, static_cast<T>((static_cast<float>(maxVal) / 100.0f) * static_cast<float>(value)), maxVal, out);
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
            return SetValue(SetValueType::Increase, static_cast<T>((static_cast<float>(maxVal) / 100.0f) * static_cast<float>(value)), maxVal, out);
        }
        return false;
    }
public:
    ResourceComp() = delete;
    explicit ResourceComp(Actor& owner) :
        owner_(owner),
        energy_(0.0f),
        health_(0.0f),
        adrenaline_(0.0f),
        overcast_(0.0f),
        healthRegen_(1.0f),
        energyRegen_(2.0f),
        dirtyFlags_(0)
    { }
    ~ResourceComp() = default;

    int GetHealth() const { return static_cast<int>(health_); }
    void SetHealth(SetValueType t, int value);
    int GetEnergy() const { return static_cast<int>(energy_); }
    void SetEnergy(SetValueType t, int value);
    int GetAdrenaline() const { return static_cast<int>(adrenaline_); }
    void SetAdrenaline(SetValueType t, int value);
    int GetOvercast() const { return static_cast<int>(overcast_); }
    void SetOvercast(SetValueType t, int value);
    int GetHealthRegen() const { return static_cast<int>(healthRegen_); }
    void SetHealthRegen(SetValueType t, int value);
    int GetEnergyRegen() const { return static_cast<int>(energyRegen_); }
    void SetEnergyRegen(SetValueType t, int value);
    int GetMaxHealth() const { return maxHealth_; }
    void SetMaxHealth(int value);
    int GetMaxEnergy() const { return maxEnergy_; }
    void SetMaxEnergy(int value);

    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message, bool ignoreDirty = false);
};

}
}

