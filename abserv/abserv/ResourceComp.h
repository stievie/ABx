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
    Decrease
};

enum ResourceDirty
{
    DirtyEnergy = 1,
    DirtyHealth = 1 << 1,
    DirtyAdrenaline = 1 << 2,
    DirtyOvercast = 1 << 3,
    DirtyHealthRegen = 1 << 4,
    DirtyEnergyRegen = 1 << 5,
};

class ResourceComp
{
private:
    Actor& owner_;
    float energy_;
    float health_;
    float adrenaline_;
    float overcast_;
    float healthRegen_;
    float energyRegen_;
    uint32_t dirtyFlags_;
    bool SetValue(SetValueType t, float value, float& out)
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
            if (!Math::Equals(value, 0.0f))
            {
                out -= value;
                return true;
            }
            break;
        case SetValueType::Increase:
            if (!Math::Equals(value, 0.0f))
            {
                out += value;
                return true;
            }
            break;
        }
        return false;
    }
public:
    ResourceComp(Actor& owner) :
        owner_(owner),
        energy_(0.0f),
        health_(0.0f),
        adrenaline_(0.0f),
        overcast_(0.0f),
        healthRegen_(0.0f),
        energyRegen_(0.0f),
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

    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message);
};

}
}

