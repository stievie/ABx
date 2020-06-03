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


#include "ResourceComp.h"
#include "Actor.h"
#include "DamageComp.h"
#include "Skill.h"
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>
#include <AB/ProtocolCodes.h>

namespace Game {
namespace Components {

ResourceComp::ResourceComp(Actor& owner) :
    owner_(owner)
{ }

bool ResourceComp::IncreaseMorale()
{
    bool result{ false };
    if (morale_ < MAX_MORALE)
    {
        morale_ += 2;
        morale_ = Math::Clamp(morale_, MIN_MORALE, MAX_MORALE);
        dirtyFlags_ |= ResourceDirty::DirtyMorale;
        result = true;
    }
    // We should fire this event even when we reached tthe limit
    owner_.CallEvent<void(int)>(EVENT_ON_INCMORALE, morale_);
    return result;
}

bool ResourceComp::DecreaseMorale()
{
    bool result{ false };
    if (morale_ > MIN_MORALE)
    {
        morale_ -= 15;
        morale_ = Math::Clamp(morale_, MIN_MORALE, MAX_MORALE);
        dirtyFlags_ |= ResourceDirty::DirtyMorale;
        result = true;
    }
    owner_.CallEvent<void(int)>(EVENT_ON_DECMORALE, morale_);
    return result;
}

int ResourceComp::GetBaseHealth() const
{
    const unsigned levelAdvance = owner_.GetLevel() - 1;
    int hp = 100 + (static_cast<int>(levelAdvance) * 20);
    return hp;
}

void ResourceComp::UpdateResources()
{
    // Base HP
    int hp = GetBaseHealth();
    int energy = BASE_ENERGY;

    int oldHp = maxHealth_;
    int oldE = maxEnergy_;

    GetResources(owner_, hp, energy);

    SetMaxHealth(hp);
    SetMaxEnergy(energy);
    if (maxHealth_ != oldHp)
        dirtyFlags_ |= ResourceDirty::DirtyMaxHealth;
    if (maxEnergy_ != oldE)
        dirtyFlags_ |= ResourceDirty::DirtyMaxEnergy;
}

void ResourceComp::SetHealth(SetValueType t, int value)
{
    const float oldVal = health_;
    if (SetValue(t, static_cast<float>(value), static_cast<float>(maxHealth_), health_))
    {
        dirtyFlags_ |= ResourceDirty::DirtyHealth;
        if (health_ < oldVal)
            lastHpDecrease_ = Utils::Tick();
    }
}

void ResourceComp::SetEnergy(SetValueType t, int value)
{
    if (SetValue(t, static_cast<float>(value), static_cast<float>(maxEnergy_), energy_))
        dirtyFlags_ |= ResourceDirty::DirtyEnergy;
}

void ResourceComp::SetAdrenaline(SetValueType t, int value)
{
    if (SetValue(t, static_cast<float>(value), 0.0f, adrenaline_))
        dirtyFlags_ |= ResourceDirty::DirtyAdrenaline;
}

void ResourceComp::SetOvercast(SetValueType t, int value)
{
    if (SetValue(t, static_cast<float>(value), static_cast<float>(maxEnergy_), overcast_))
        dirtyFlags_ |= ResourceDirty::DirtyOvercast;
}

void ResourceComp::SetHealthRegen(SetValueType t, int value)
{
    if (SetValue(t, static_cast<float>(value), static_cast<float>(MAX_HEALTH_REGEN), healthRegen_))
        dirtyFlags_ |= ResourceDirty::DirtyHealthRegen;
}

void ResourceComp::SetEnergyRegen(SetValueType t, int value)
{
    if (SetValue(t, static_cast<float>(value), MAX_ENERGY_REGEN, energyRegen_))
        dirtyFlags_ |= ResourceDirty::DirtyEnergyRegen;
}

void ResourceComp::SetMaxHealth(int value)
{
    if (maxHealth_ != value)
    {
        maxHealth_ = value;
        dirtyFlags_ |= ResourceDirty::DirtyMaxHealth;
    }
}

void ResourceComp::SetMaxEnergy(int value)
{
    if (maxEnergy_ != value)
    {
        maxEnergy_ = value;
        dirtyFlags_ |= ResourceDirty::DirtyMaxEnergy;
    }
}

int ResourceComp::GetValue(ResourceType type) const
{
    switch (type)
    {
    case ResourceType::Energy:
        return GetEnergy();
    case ResourceType::Health:
        return GetHealth();
    case ResourceType::Adrenaline:
        return GetAdrenaline();
    case ResourceType::Overcast:
        return GetOvercast();
    case ResourceType::HealthRegen:
        return GetHealthRegen();
    case ResourceType::EnergyRegen:
        return GetEnergyRegen();
    case ResourceType::MaxHealth:
        return GetMaxHealth();
    case ResourceType::MaxEnergy:
        return GetMaxEnergy();
    default:
        return 0;
    }
}

void ResourceComp::SetValue(ResourceType type, SetValueType t, int value)
{
    switch (type)
    {
    case ResourceType::None:
        break;
    case ResourceType::Energy:
        SetEnergy(t, value);
        break;
    case ResourceType::Health:
        SetHealth(t, value);
        break;
    case ResourceType::Adrenaline:
        SetAdrenaline(t, value);
        break;
    case ResourceType::Overcast:
        SetOvercast(t, value);
        break;
    case ResourceType::HealthRegen:
        SetHealthRegen(t, value);
        break;
    case ResourceType::EnergyRegen:
        SetEnergyRegen(t, value);
        break;
    case ResourceType::MaxHealth:
    {
        int hp = GetMaxHealth();
        switch (t)
        {
        case SetValueType::Absolute:
            hp = value;
            break;
        case SetValueType::Increase:
            hp += value;
            break;
        case SetValueType::Decrease:
            hp -= value;
            break;
        default:
            return;
        }
        SetMaxHealth(hp);
        break;
    }
    case ResourceType::MaxEnergy:
    {
        int e = GetMaxEnergy();
        switch (t)
        {
        case SetValueType::Absolute:
            e = value;
            break;
        case SetValueType::Increase:
            e += value;
            break;
        case SetValueType::Decrease:
            e -= value;
            break;
        default:
            return;
        }
        SetMaxEnergy(e);
        break;
    }
    }
}

int ResourceComp::AddEnergy(int value)
{
    const int n = GetEnergy() + value;
    if (n > maxEnergy_)
        value -= (n + maxEnergy_);
    SetEnergy(Components::SetValueType::Increase, value);
    return value;
}

int ResourceComp::DrainEnergy(int value)
{
    const int curr = GetEnergy();
    const int result = Math::Clamp(value, 0, curr);
    SetEnergy(Components::SetValueType::Absolute, curr - result);
    return result;
}

bool ResourceComp::HaveEnoughResources(const Skill* skill) const
{
    if (!skill)
        return false;
    if (skill->adrenaline_ > GetAdrenaline())
        return false;
    if (skill->energy_ > GetEnergy())
        return false;
    if (skill->hp_ > GetHealth())
        return false;
    return true;
}

void ResourceComp::UpdateRegen(uint32_t /* timeElapsed */)
{
    // When the actor didn't get damage or lost HP for 5 seconds, increase natural HP regen by 1 every 2 seconds.
    // Maximum regen through natural regen is 7.
    // https://wiki.guildwars.com/wiki/Health
    if (!owner_.IsDead() && health_ < maxHealth_)
    {
        // Time last HP was reduced
        uint32_t last = std::min(owner_.damageComp_->NoDamageTime(),
            GetLastHpDecrease());
        // Not using skills
        last = std::min(Utils::TimeElapsed(owner_.skillsComp_->GetLastSkillTime()), last);
        // Not attacking
        last = std::min(Utils::TimeElapsed(owner_.attackComp_->GetLastAttackTime()), last);
        if (last > 5000 && healthRegen_ >= 0.0f)
        {
            if ((Utils::TimeElapsed(lastRegenIncrease_) > 2000) && GetHealthRegen() < 7)
            {
                ++naturalHealthRegen_;
                lastRegenIncrease_ = Utils::Tick();
                dirtyFlags_ |= ResourceDirty::DirtyHealthRegen;
            }
            return;
        }
    }

    // Either we lost HP or HP regen is smaller 0 (Hex or something) -> no natural regen
    if (naturalHealthRegen_ != 0)
    {
        naturalHealthRegen_ = 0;
        dirtyFlags_ |= ResourceDirty::DirtyHealthRegen;
    }
}

uint32_t ResourceComp::GetLastHpDecrease() const
{
    return Utils::TimeElapsed(lastHpDecrease_);
}

void ResourceComp::Update(uint32_t timeElapsed)
{
    if (owner_.undestroyable_ || owner_.IsDead())
        return;

    UpdateResources();
    UpdateRegen(timeElapsed);
    // 2 regen per sec
    const float sec = static_cast<float>(timeElapsed) / 1000.0f;
    // Jeder Pfeil erhöht oder senkt die Lebenspunkte um genau zwei pro Sekunde.
    if (SetValue(SetValueType::Increase, (static_cast<float>(GetHealthRegen()) * 2.0f) * sec, static_cast<float>(maxHealth_), health_))
        dirtyFlags_ |= ResourceDirty::DirtyHealth;
    // Also bedeutet 1 Pfeil eine Regeneration (oder Degeneration) von 0,33 Energiepunkten pro Sekunde.
    if (SetValue(SetValueType::Increase, (energyRegen_ * 0.33f) * sec, static_cast<float>(maxEnergy_), energy_))
        dirtyFlags_ |= ResourceDirty::DirtyEnergy;
    // Überzaubert wird alle drei Sekunden um einen Punkt abgebaut
    if (overcast_ > 0.0f)
    {
        if (SetValue(SetValueType::Decrease, (1.0f / 3.0f) * sec, static_cast<float>(maxEnergy_), overcast_))
            dirtyFlags_ |= ResourceDirty::DirtyOvercast;
    }
    else if (overcast_ < 0.0f)
        overcast_ = 0.0f;

    if (health_ <= 0.0f && !owner_.IsDead())
        owner_.Die(nullptr);
}

void ResourceComp::Write(Net::NetworkMessage& message, bool ignoreDirty /* = false */)
{
    if (!ignoreDirty && dirtyFlags_ == 0)
        return;

    if (ignoreDirty || (dirtyFlags_ & ResourceDirty::DirtyHealth) == ResourceDirty::DirtyHealth)
    {
        message.AddByte(AB::GameProtocol::ServerPacketType::ObjectResourceChanged);
        AB::Packets::Server::ObjectResourceChanged packet = {
            owner_.id_,
            static_cast<uint8_t>(AB::GameProtocol::ResourceType::Health),
            static_cast<int16_t>(health_)
        };
        AB::Packets::Add(packet, message);
    }
    if (ignoreDirty || (dirtyFlags_ & ResourceDirty::DirtyEnergy) == ResourceDirty::DirtyEnergy)
    {
        message.AddByte(AB::GameProtocol::ServerPacketType::ObjectResourceChanged);
        AB::Packets::Server::ObjectResourceChanged packet = {
            owner_.id_,
            static_cast<uint8_t>(AB::GameProtocol::ResourceType::Energy),
            static_cast<int16_t>(energy_)
        };
        AB::Packets::Add(packet, message);
    }
    if (ignoreDirty || (dirtyFlags_ & ResourceDirty::DirtyAdrenaline) == ResourceDirty::DirtyAdrenaline)
    {
        message.AddByte(AB::GameProtocol::ServerPacketType::ObjectResourceChanged);
        AB::Packets::Server::ObjectResourceChanged packet = {
            owner_.id_,
            static_cast<uint8_t>(AB::GameProtocol::ResourceType::Adrenaline),
            static_cast<int16_t>(adrenaline_)
        };
        AB::Packets::Add(packet, message);
    }
    if (ignoreDirty || (dirtyFlags_ & ResourceDirty::DirtyOvercast) == ResourceDirty::DirtyOvercast)
    {
        message.AddByte(AB::GameProtocol::ServerPacketType::ObjectResourceChanged);
        AB::Packets::Server::ObjectResourceChanged packet = {
            owner_.id_,
            static_cast<uint8_t>(AB::GameProtocol::ResourceType::Overcast),
            static_cast<int16_t>(overcast_)
        };
        AB::Packets::Add(packet, message);
    }
    if (ignoreDirty || (dirtyFlags_ & ResourceDirty::DirtyHealthRegen) == ResourceDirty::DirtyHealthRegen)
    {
        message.AddByte(AB::GameProtocol::ServerPacketType::ObjectResourceChanged);
        AB::Packets::Server::ObjectResourceChanged packet = {
            owner_.id_,
            static_cast<uint8_t>(AB::GameProtocol::ResourceType::HealthRegen),
            static_cast<int16_t>(GetHealthRegen())
        };
        AB::Packets::Add(packet, message);
    }
    if (ignoreDirty || (dirtyFlags_ & ResourceDirty::DirtyEnergyRegen) == ResourceDirty::DirtyEnergyRegen)
    {
        message.AddByte(AB::GameProtocol::ServerPacketType::ObjectResourceChanged);
        AB::Packets::Server::ObjectResourceChanged packet = {
            owner_.id_,
            static_cast<uint8_t>(AB::GameProtocol::ResourceType::EnergyRegen),
            static_cast<int16_t>(energyRegen_)
        };
        AB::Packets::Add(packet, message);
    }
    if (ignoreDirty || (dirtyFlags_ & ResourceDirty::DirtyMaxHealth) == ResourceDirty::DirtyMaxHealth)
    {
        message.AddByte(AB::GameProtocol::ServerPacketType::ObjectResourceChanged);
        AB::Packets::Server::ObjectResourceChanged packet = {
            owner_.id_,
            static_cast<uint8_t>(AB::GameProtocol::ResourceType::MaxHealth),
            static_cast<int16_t>(maxHealth_)
        };
        AB::Packets::Add(packet, message);
    }
    if (ignoreDirty || (dirtyFlags_ & ResourceDirty::DirtyMaxEnergy) == ResourceDirty::DirtyMaxEnergy)
    {
        message.AddByte(AB::GameProtocol::ServerPacketType::ObjectResourceChanged);
        AB::Packets::Server::ObjectResourceChanged packet = {
            owner_.id_,
            static_cast<uint8_t>(AB::GameProtocol::ResourceType::MaxEnergy),
            static_cast<int16_t>(maxEnergy_)
        };
        AB::Packets::Add(packet, message);
    }
    if (ignoreDirty || (dirtyFlags_ & ResourceDirty::DirtyMorale) == ResourceDirty::DirtyMorale)
    {
        message.AddByte(AB::GameProtocol::ServerPacketType::ObjectResourceChanged);
        AB::Packets::Server::ObjectResourceChanged packet = {
            owner_.id_,
            static_cast<uint8_t>(AB::GameProtocol::ResourceType::Morale),
            static_cast<int16_t>(morale_)
        };
        AB::Packets::Add(packet, message);
    }

    if (!ignoreDirty)
        dirtyFlags_ = 0;
}

}
}
