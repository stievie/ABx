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


#include "ProgressComp.h"
#include "Player.h"
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>

namespace Game {
namespace Components {

ProgressComp::ProgressComp(Actor& owner) :
    owner_(owner)
{
    owner_.SubscribeEvent<void(Actor*, Actor*)>(EVENT_ON_DIED, std::bind(&ProgressComp::OnDied, this, std::placeholders::_1, std::placeholders::_2));
    owner_.SubscribeEvent<void(Actor*, Actor*)>(EVENT_ON_KILLEDFOE, std::bind(&ProgressComp::OnKilledFoe,
        this, std::placeholders::_1, std::placeholders::_2));
}

void ProgressComp::Write(Net::NetworkMessage& message)
{
    if (items_.size() == 0)
        return;

    for (const auto& i : items_)
    {
        switch (i.type)
        {
        case ProgressType::XPIncrease:
        {
            message.AddByte(AB::GameProtocol::ServerPacketType::GameObjectProgress);
            AB::Packets::Server::ObjectProgress packet = {
                owner_.id_,
                static_cast<uint8_t>(AB::GameProtocol::ObjectProgressType::XPIncreased),
                static_cast<int16_t>(i.value)
            };
            AB::Packets::Add(packet, message);
            break;
        }
        case ProgressType::GotSkillPoint:
        {
            message.AddByte(AB::GameProtocol::ServerPacketType::GameObjectProgress);
            AB::Packets::Server::ObjectProgress packet = {
                owner_.id_,
                static_cast<uint8_t>(AB::GameProtocol::ObjectProgressType::GotSkillPoint),
                static_cast<int16_t>(i.value)
            };
            AB::Packets::Add(packet, message);
            break;
        }
        case ProgressType::LevelAdvance:
        {
            message.AddByte(AB::GameProtocol::ServerPacketType::GameObjectProgress);
            AB::Packets::Server::ObjectProgress packet = {
                owner_.id_,
                static_cast<uint8_t>(AB::GameProtocol::ObjectProgressType::LevelAdvance),
                static_cast<int16_t>(i.value)
            };
            AB::Packets::Add(packet, message);
            break;
        }
        case ProgressType::TitleAdvance:
            // TODO:
            break;
        case ProgressType::AttributePointGain:
            message.AddByte(AB::GameProtocol::ServerPacketType::GameObjectProgress);
            AB::Packets::Server::ObjectProgress packet = {
                owner_.id_,
                static_cast<uint8_t>(AB::GameProtocol::ObjectProgressType::AttribPointsGain),
                static_cast<int16_t>(i.value)
            };
            AB::Packets::Add(packet, message);
        }
    }
    items_.clear();
}

void ProgressComp::OnKilledFoe(Actor* foe, Actor*)
{
    // Get only XP for the first death of NPCs.
    if (foe && !owner_.IsDead() && foe->GetDeaths() == 0)
        // When the owner is not dead add some XP for killing this foe
        owner_.progressComp_->AddXpForKill(foe);
}

void ProgressComp::OnDied(Actor*, Actor* killer)
{
    if (Is<Player>(owner_))
    {
        auto& player = To<Player>(owner_);
        player.deathStats_[AB::Entities::DeathStatIndexCount] =
            player.deathStats_[AB::Entities::DeathStatIndexCount].GetInt() + 1;
        player.deathStats_[AB::Entities::DeathStatIndexAtXp] = player.GetXp();
    }
    // When we die all Enemies in range get XP for that
    owner_.VisitEnemiesInRange(Ranges::Aggro, [&](const Actor& actor)
    {
        const_cast<Actor&>(actor).CallEvent<void(Actor*, Actor*)>(EVENT_ON_KILLEDFOE, &owner_, killer);
        return Iteration::Continue;
    });
    ++deaths_;
}

void ProgressComp::AddXp(int value)
{
    // Max XP
    if (value > static_cast<int>(SKILLPOINT_ADVANCE_XP))
        value = static_cast<int>(SKILLPOINT_ADVANCE_XP);

    const uint32_t oldXp = owner_.GetXp();
    owner_.AddXp(value);
    items_.push_back({ ProgressType::XPIncrease, value });

    const uint32_t level = owner_.GetLevel();
    if (level < LEVEL_CAP)
    {
        const uint32_t nextLevel = std::max(level * 600 + 1400, SKILLPOINT_ADVANCE_XP);
        if (owner_.GetXp() >= nextLevel)
        {
            AdvanceLevel();
            AddSkillPoint();
        }
    }
    else
    {
        const uint32_t x = oldXp / SKILLPOINT_ADVANCE_XP;
        const uint32_t y = owner_.GetXp() / SKILLPOINT_ADVANCE_XP;
        const uint32_t z = y - x;
        if (z > 0)
            AddSkillPoint();
    }
}

void ProgressComp::AddXpForKill(Actor* victim)
{
    const size_t allyCount = owner_.GetAllyCountInRange(Ranges::Aggro);
    const int a = static_cast<int>(victim->GetLevel());
    const int b = static_cast<int>(owner_.GetLevel());
    const int xp = (100 + 4 * std::abs(a - b) + 16 * (a - b)) / static_cast<int>(allyCount);
    AddXp(xp);
}

void ProgressComp::AddSkillPoint()
{
    items_.push_back({ ProgressType::GotSkillPoint, 1 });
    owner_.AddSkillPoint();
}

void ProgressComp::AdvanceLevel()
{
    if (owner_.GetLevel() >= LEVEL_CAP)
        // Max level reached
        return;

    const uint32_t oldLevel = owner_.GetLevel();
    const uint32_t oldAttribPoints = owner_.GetAttributePoints();
    items_.push_back({ ProgressType::LevelAdvance, 1 });
    owner_.AdvanceLevel();
    if (oldLevel >= 2 && oldLevel < LEVEL_CAP)
    {
        const int attribPointsChange = static_cast<int>(owner_.GetAttributePoints()) - static_cast<int>(oldAttribPoints);
        if (attribPointsChange > 0)
            items_.push_back({ ProgressType::AttributePointGain, attribPointsChange });
    }
}

}
}
