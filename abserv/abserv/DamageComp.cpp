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


#include "DamageComp.h"
#include "Actor.h"
#include <sa/WeightedSelector.h>
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>

namespace Game {
namespace Components {

DamageComp::DamageComp(Actor& owner) :
    owner_(owner)
{ }

void DamageComp::ApplyDamage(Actor* source, uint32_t index, DamageType type, int value, float penetration, bool melee)
{
    if (owner_.IsDead())
        return;

    lastDamage_ = Utils::Tick();
    // Get a random pos where to hit
    const DamagePos pos = GetDamagePos();
    // Get the armor effect at this pos with the given damage type and armor penetration
    const float am = owner_.GetArmorEffect(type, pos, penetration);
    const int realValue = static_cast<int>(static_cast<float>(value) * am);
    damages_.Enqueue({ { lastDamage_, type, pos, realValue, (source ? source->id_ : 0), index }, true });
    owner_.resourceComp_->SetHealth(SetValueType::Decrease, abs(realValue));
    if (source)
    {
        lastDamager_ = source->GetPtr<Actor>();
        if (melee)
        {
            lastMeleeDamager_ = source->GetPtr<Actor>();
            lastMeleeDamage_ = lastDamage_;
        }
    }
}

int DamageComp::DrainLife(Actor* source, uint32_t index, int value)
{
    if (owner_.IsDead())
        return 0;

    const int currLife = owner_.resourceComp_->GetHealth();
    const int result = Math::Clamp(value, 0, currLife);
    lastDamage_ = Utils::Tick();
    damages_.Enqueue({ { lastDamage_, DamageType::LifeDrain, DamagePos::NoPos, result, (source ? source->id_ : 0), index }, true });
    owner_.resourceComp_->SetHealth(Components::SetValueType::Absolute, currLife - result);
    if (source)
        lastDamager_ = source->GetPtr<Actor>();
    return result;
}

void DamageComp::Touch()
{
    lastDamage_ = Utils::Tick();
}

DamagePos DamageComp::GetDamagePos() const
{
    static sa::WeightedSelector<DamagePos> ws;
    if (!ws.IsInitialized())
    {
        for (size_t i = 0; i < Utils::CountOf(DamagePosChances); ++i)
            ws.Add(static_cast<DamagePos>(i), DamagePosChances[i]);
        ws.Update();
    }

    auto* rng = GetSubsystem<Crypto::Random>();
    const float rnd1 = rng->GetFloat();
    const float rnd2 = rng->GetFloat();
    return ws.Get(rnd1, rnd2);
}

uint32_t DamageComp::NoDamageTime() const
{
    return Utils::TimeElapsed(lastDamage_);
}

bool DamageComp::IsGettingMeleeDamage() const
{
    return Utils::TimeElapsed(lastMeleeDamage_) <= 1000;
}

bool DamageComp::IsLastDamager(const Actor& actor)
{
    if (auto d = lastDamager_.lock())
        if (actor.id_ == d->id_)
            return true;
    return false;
}

void DamageComp::Write(Net::NetworkMessage& message)
{
    if (damages_.IsEmpty())
        return;
    for (auto& d : damages_)
    {
        if (!d.dirty)
            continue;

        message.AddByte(AB::GameProtocol::ServerPacketType::ObjectDamaged);
        AB::Packets::Server::ObjectDamaged packet = {
            owner_.id_,
            d.damage.actorId,
            static_cast<uint16_t>(d.damage.index),
            static_cast<uint8_t>(d.damage.type),
            static_cast<uint16_t>(abs(d.damage.value))
        };
        AB::Packets::Add(packet, message);
        d.dirty = false;
    }
}

bool DamageComp::GotDamageType(DamageType type) const
{
    if (damages_.IsEmpty())
        return false;
    for (const auto& d : damages_)
    {
        if (Utils::TimeElapsed(d.damage.tick) > LAST_DAMAGE_TIME)
            continue;
        if (d.damage.type == type)
            return true;
    }
    return false;
}

bool DamageComp::GotDamageCategory(DamageTypeCategory cat) const
{
    if (damages_.IsEmpty())
        return false;

    if (cat == DamageTypeCategory::Any)
    {
        const auto& last = damages_.Last();
        return Utils::TimeElapsed(last.damage.tick) <= LAST_DAMAGE_TIME;
    }

    for (const auto& d : damages_)
    {
        if (Utils::TimeElapsed(d.damage.tick) > LAST_DAMAGE_TIME)
            continue;
        if (IsDamageCategory(d.damage.type, cat))
            return true;
    }
    return false;
}

}
}
