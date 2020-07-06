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

#include "Actor.h"
#include "AiComp.h"
#include "AiLoader.h"
#include "Chat.h"
#include "TriggerComp.h"
#include "WanderComp.h"
#include <eastl.hpp>
#include <sa/Bits.h>
#include <set>

namespace Game {

class Map;
class Crowd;

class Npc final : public Actor
{
private:
    enum Function : uint32_t
    {
        FunctionNone = 0,
        FunctionUpdate = 1,
        FunctionOnTrigger = 1 << 1,
        FunctionOnLeftArea = 1 << 2,
        FunctionOnGetQuote = 1 << 3
    };

    /// This NPC exists only on the server, i.e. is not spawned on the client, e.g. a trigger box.
    bool serverOnly_{ false };
    uint32_t level_{ 1 };
    uint32_t itemIndex_{ 0 };
    AB::Entities::CharacterSex sex_{ AB::Entities::CharacterSex::Unknown };
    /// Quests this NPC may have for the player
    ea::set<uint32_t> quests_;
    ea::set<AB::Entities::ItemType> sellItemTypes_;
    uint32_t functions_{ FunctionNone };
    bool HaveFunction(Function func) const
    {
        return luaInitialized_ && sa::bits::is_set(functions_, func);
    }
    kaguya::State luaState_;
    bool luaInitialized_;
    void InitializeLua();
    std::string GetQuote(int index);
    void _LuaAddWanderPoint(const Math::StdVector3& point);
    void _LuaAddWanderPoints(const std::vector<Math::StdVector3>& points);
    /// Set the name of the NPC. This must happen before the spawn data is sent to the clients
    void _LuaSetName(const std::string& name);
    void _LuaAddQuest(uint32_t index);
private:
    // Events
    void OnArrived();
    void OnInterruptedAttack();
    void OnInterruptedSkill(Skill* skill);
    void OnKnockedDown(uint32_t time);
    void OnHealed(int hp);
    void OnResurrected(int health, int energy);
    void OnClicked(Actor* selector);
    void OnCollide(GameObject* other);
    void OnSelected(Actor* selector);
    void OnTrigger(GameObject* other);
    void OnLeftArea(GameObject* other);
    void OnDied(Actor*, Actor*);
    void OnEndUseSkill(Skill* skill);
    void OnStartUseSkill(Skill* skill);
    void OnAttack(Actor* target, bool& canAttack);
    void OnGettingAttacked(Actor* source, bool& canGetAttacked);
    void OnAttacked(Actor* source, DamageType type, int32_t damage, bool& canGetAttacked);
    void OnInterruptingAttack(bool& success);
    void OnInterruptingSkill(AB::Entities::SkillType type, Skill* skill, bool& success);
    void OnUseSkill(Actor* target, Skill* skill, bool& success);
    void OnSkillTargeted(Actor* source, Skill* skill, bool& succcess);
public:
    static void RegisterLua(kaguya::State& state);

    Npc();
    ~Npc() override;

    bool LoadScript(const std::string& fileName);
    AB::GameProtocol::GameObjectType GetType() const override
    {
        return AB::GameProtocol::GameObjectType::Npc;
    }

    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;

    uint32_t GetLevel() const override { return level_; }
    bool CanAttack() const override { return true; }
    bool CanUseSkill() const override { return true; }
    void SetLevel(uint32_t value) override;
    uint32_t GetItemIndex() const override
    {
        return itemIndex_;
    }
    AB::Entities::CharacterSex GetSex() const override
    {
        return sex_;
    }
    bool SetBehavior(const std::string& name);
    float GetAggro(const Actor* other);
    int GetBestSkillIndex(SkillEffect effect, SkillEffectTarget target,
        AB::Entities::SkillType interrupts = AB::Entities::SkillTypeAll,
        const Actor* targetActor = nullptr);
    bool GetSkillCandidates(
        ea::vector<int>& results,
        SkillEffect effect, SkillEffectTarget target,
        AB::Entities::SkillType interrupts = AB::Entities::SkillTypeAll,
        const Actor* targetActor = nullptr);
    bool IsServerOnly() const { return serverOnly_; }
    void SetServerOnly(bool value) { serverOnly_ = value; }
    bool IsWander() const;
    void SetWander(bool value);

    void WriteSpawnData(Net::NetworkMessage& msg) override;

    void Say(ChatType channel, const std::string& message);
    bool SayQuote(ChatType channel, int index);
    void Whisper(Player* player, const std::string& message);
    /// Shooting a projectile without having a weapon that can spawn projectiles
    void ShootAt(const std::string& itemUuid, Actor* target);
    ea::set<uint32_t> GetQuestsForPlayer(const Player& player) const;
    bool HaveQuestsForPlayer(const Player& player) const;
    bool IsSellingItemType(AB::Entities::ItemType type) const;

    ea::unique_ptr<Components::AiComp> aiComp_;
    ea::unique_ptr<Components::WanderComp> wanderComp_;
};

template <>
inline bool Is<Npc>(const GameObject& obj)
{
    return obj.GetType() == AB::GameProtocol::GameObjectType::Npc;
}

}
