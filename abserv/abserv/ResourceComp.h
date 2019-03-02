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
    int32_t maxHealth_;
    int32_t maxEnergy_;
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
                return false;
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
                return false;
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

    int16_t GetHealth() const { return static_cast<int16_t>(health_); }
    void SetHealth(SetValueType t, int16_t value);
    int16_t GetEnergy() const { return static_cast<int16_t>(energy_); }
    void SetEnergy(SetValueType t, int16_t value);
    int16_t GetAdrenaline() const { return static_cast<int16_t>(adrenaline_); }
    void SetAdrenaline(SetValueType t, int16_t value);
    int16_t GetOvercast() const { return static_cast<int16_t>(overcast_); }
    void SetOvercast(SetValueType t, int16_t value);
    int8_t GetHealthRegen() const { return static_cast<int8_t>(healthRegen_); }
    void SetHealthRegen(SetValueType t, int8_t value);
    int8_t GetEnergyRegen() const { return static_cast<int8_t>(energyRegen_); }
    void SetEnergyRegen(SetValueType t, int8_t value);
    int16_t GetMaxHealth() const { return static_cast<int16_t>(maxHealth_); }
    void SetMaxHealth(int16_t value);
    int16_t GetMaxEnergy() const { return static_cast<int16_t>(maxEnergy_); }
    void SetMaxEnergy(int16_t value);

    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message, bool ignoreDirty = false);
};

}
}

