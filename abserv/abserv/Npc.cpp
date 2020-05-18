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

#include "stdafx.h"
#include "Npc.h"
#include "Crowd.h"
#include "DamageComp.h"
#include "DataProvider.h"
#include "GameManager.h"
#include "Group.h"
#include "Player.h"
#include "QuestComp.h"
#include "ScriptManager.h"
#include <abai/BevaviorCache.h>
#include <Mustache/mustache.hpp>

namespace Game {

void Npc::InitializeLua()
{
    Lua::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
    luaInitialized_ = true;
}

void Npc::RegisterLua(kaguya::State& state)
{
    state["Npc"].setClass(kaguya::UserdataMetatable<Npc, Actor>()
        .addFunction("IsServerOnly", &Npc::IsServerOnly)
        .addFunction("SetServerOnly", &Npc::SetServerOnly)

        .addFunction("SetLevel", &Npc::SetLevel)     // Can only be used in onInit(), i.e. before it is sent to the clients
        .addFunction("SetName", &Npc::_LuaSetName)   // After the spawn has been sent to the client it can not be changed
        .addFunction("Say", &Npc::Say)
        .addFunction("SayQuote", &Npc::SayQuote)
        .addFunction("ShootAt", &Npc::ShootAt)
        .addFunction("SetBehavior", &Npc::SetBehavior)
        .addFunction("IsWander", &Npc::IsWander)
        .addFunction("SetWander", &Npc::SetWander)
        .addFunction("AddWanderPoint", &Npc::_LuaAddWanderPoint)
        .addFunction("AddWanderPoints", &Npc::_LuaAddWanderPoints)
        .addFunction("AddQuest", &Npc::_LuaAddQuest)
    );
}

Npc::Npc() :
    Actor(),
    luaInitialized_(false)
{
    events_.Subscribe<void(Actor*, bool&)>(EVENT_ON_ATTACK, std::bind(&Npc::OnAttack, this, std::placeholders::_1, std::placeholders::_2));
    events_.Subscribe<void(Actor*, bool&)>(EVENT_ON_GETTING_ATTACKED, std::bind(&Npc::OnGettingAttacked, this, std::placeholders::_1, std::placeholders::_2));
    events_.Subscribe<void(Actor*, DamageType, int32_t, bool&)>(EVENT_ON_ATTACKED, std::bind(
        &Npc::OnAttacked, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    events_.Subscribe<void(bool&)>(EVENT_ON_INTERRUPTING_ATTACK, std::bind(&Npc::OnInterruptingAttack, this, std::placeholders::_1));
    events_.Subscribe<void(AB::Entities::SkillType, Skill*, bool&)>(EVENT_ON_INTERRUPTING_SKILL,
        std::bind(&Npc::OnInterruptingSkill, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    events_.Subscribe<void(Actor*, Skill*, bool&)>(EVENT_ON_SKILLTARGETED, std::bind(&Npc::OnSkillTargeted, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    events_.Subscribe<void(Actor*, Skill*, bool&)>(EVENT_ON_USESKILL, std::bind(&Npc::OnUseSkill, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    events_.Subscribe<void(Actor*, Actor*)>(EVENT_ON_DIED, std::bind(&Npc::OnDied, this, std::placeholders::_1, std::placeholders::_2));
    events_.Subscribe<void(Skill*)>(EVENT_ON_ENDUSESKILL, std::bind(&Npc::OnEndUseSkill, this, std::placeholders::_1));
    events_.Subscribe<void(Skill*)>(EVENT_ON_STARTUSESKILL, std::bind(&Npc::OnStartUseSkill, this, std::placeholders::_1));
    events_.Subscribe<void(Actor*)>(EVENT_ON_CLICKED, std::bind(&Npc::OnClicked, this, std::placeholders::_1));
    events_.Subscribe<void(GameObject*)>(EVENT_ON_COLLIDE, std::bind(&Npc::OnCollide, this, std::placeholders::_1));
    events_.Subscribe<void(Actor*)>(EVENT_ON_SELECTED, std::bind(&Npc::OnSelected, this, std::placeholders::_1));
    events_.Subscribe<void(GameObject*)>(EVENT_ON_TRIGGER, std::bind(&Npc::OnTrigger, this, std::placeholders::_1));
    events_.Subscribe<void(GameObject*)>(EVENT_ON_LEFTAREA, std::bind(&Npc::OnLeftArea, this, std::placeholders::_1));
    events_.Subscribe<void(void)>(EVENT_ON_ARRIVED, std::bind(&Npc::OnArrived, this));
    events_.Subscribe<void(void)>(EVENT_ON_INTERRUPTEDATTACK, std::bind(&Npc::OnInterruptedAttack, this));
    events_.Subscribe<void(Skill*)>(EVENT_ON_INTERRUPTEDSKILL, std::bind(&Npc::OnInterruptedSkill, this, std::placeholders::_1));
    events_.Subscribe<void(uint32_t)>(EVENT_ON_KNOCKED_DOWN, std::bind(&Npc::OnKnockedDown, this, std::placeholders::_1));
    events_.Subscribe<void(int)>(EVENT_ON_HEALED, std::bind(&Npc::OnHealed, this, std::placeholders::_1));
    events_.Subscribe<void(int, int)>(EVENT_ON_RESURRECTED, std::bind(&Npc::OnResurrected, this, std::placeholders::_1, std::placeholders::_2));
    // Party and Groups must be unique, i.e. share the same ID pool.
    groupId_ = Group::GetNewId();
    InitializeLua();
}

Npc::~Npc()
{
    auto* group = GetGroup();
    if (group)
        group->Remove(id_);
}

bool Npc::LoadScript(const std::string& fileName)
{
    script_ = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(fileName);
    if (!script_)
        return false;
    if (!script_->Execute(luaState_))
        return false;

    name_ = static_cast<const char*>(luaState_["name"]);
    level_ = luaState_["level"];
    itemIndex_ = luaState_["itemIndex"];
    if (Lua::IsNumber(luaState_, "sex"))
        sex_ = luaState_["sex"];
    if (Lua::IsNumber(luaState_, "group_id"))
        groupId_ = luaState_["group_id"];
    if (Lua::IsBool(luaState_, "wander"))
        SetWander(luaState_("wander"));

    if (Lua::IsNumber(luaState_, "creatureState"))
        stateComp_.SetState(luaState_["creatureState"], true);
    else
        stateComp_.SetState(AB::GameProtocol::CreatureState::Idle, true);

    IO::DataClient* client = GetSubsystem<IO::DataClient>();

    if (Lua::IsNumber(luaState_, "prof1Index"))
    {
        skills_->prof1_.index = luaState_["prof1Index"];
        if (skills_->prof1_.index != 0)
        {
            if (!client->Read(skills_->prof1_))
            {
                LOG_WARNING << "Unable to read primary profession of " << GetName() << ", index = " << skills_->prof1_.index << std::endl;
            }
        }
    }
    if (Lua::IsNumber(luaState_, "prof2Index"))
    {
        skills_->prof2_.index = luaState_["prof2Index"];
        if (skills_->prof2_.index != 0)
        {
            if (!client->Read(skills_->prof2_))
            {
                LOG_WARNING << "Unable to read secondary profession of " << GetName() << ", index = " << skills_->prof2_.index << std::endl;
            }
        }
    }

    std::string bt;
    if (Lua::IsString(luaState_, "behavior"))
        bt = static_cast<const char*>(luaState_["behavior"]);
    if (Lua::IsFunction(luaState_, "onUpdate"))
        sa::bits::set(functions_, FunctionUpdate);
    if (Lua::IsFunction(luaState_, "onTrigger"))
        sa::bits::set(functions_, FunctionOnTrigger);
    if (Lua::IsFunction(luaState_, "onLeftArea"))
        sa::bits::set(functions_, FunctionOnLeftArea);
    if (Lua::IsFunction(luaState_, "onGetQuote"))
        sa::bits::set(functions_, FunctionOnGetQuote);

    GetSkillBar()->InitAttributes();
    // Initialize resources, etc. may be overwritten in onInit() in the NPC script bellow.
    Initialize();

    if (!bt.empty())
        SetBehavior(bt);

    return luaState_["onInit"]();
}

void Npc::SetLevel(uint32_t value)
{
    level_ = value;
    resourceComp_->UpdateResources();
}

void Npc::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    // I think first we should run the BT
    if (aiComp_)
        aiComp_->Update(timeElapsed);
    if (wanderComp_)
        wanderComp_->Update(timeElapsed);

    Actor::Update(timeElapsed, message);

    if (luaInitialized_ && HaveFunction(FunctionUpdate))
        luaState_["onUpdate"](timeElapsed);

    Lua::CollectGarbage(luaState_);
}

bool Npc::SetBehavior(const std::string& name)
{
    auto* cache = GetSubsystem<AI::BevaviorCache>();
    auto root = cache->Get(name);
    if (!root)
    {
        aiComp_.reset();
        if (!name.empty())
            LOG_WARNING << "Behavior with name " << name << " not found in cache" << std::endl;
        return false;
    }
    aiComp_ = ea::make_unique<Components::AiComp>(*this);
    aiComp_->GetAgent().SetBehavior(root);
    return true;
}

float Npc::GetAggro(const Actor* other)
{
    if (!other || !IsEnemy(other))
        return 0.0f;

    auto* random = GetSubsystem<Crypto::Random>();
    const float dist = 1.0f / (GetPosition().Distance(other->GetPosition()) / RANGE_AGGRO);
    const float health = 1.0f - other->resourceComp_->GetHealthRatio();
    const float ld = damageComp_->IsLastDamager(*other) ? 2.0f : 1.0f;
    const float rval = random->GetFloat();
    return ((dist + health) * ld) * rval;
}

bool Npc::GetSkillCandidates(
    ea::vector<int>& results,
    SkillEffect effect, SkillEffectTarget target,
    AB::Entities::SkillType interrupts /* = AB::Entities::SkillTypeAll */,
    const Actor* targetActor /* = nullptr */)
{
    // skill index -> cost (smaller is better)
    ea::map<int, float> sorting;
    skills_->VisitSkills([&](int index, const Skill& current)
    {
        if (!current.CanUseOnTarget(*this, targetActor))
            return Iteration::Continue;
        if (current.NeedsTarget())
        {
            // If there is a target check if it's in range
            if (targetActor && !IsInRange(current.GetRange(), targetActor))
                return Iteration::Continue;
        }
        if (!current.HasEffect(effect))
            return Iteration::Continue;
        if (target != SkillTargetNone && !current.HasTarget(target))
            return Iteration::Continue;
        if (!current.IsRecharged())
            return Iteration::Continue;
        if (!resourceComp_->HaveEnoughResources(&current))
            return Iteration::Continue;
        if (effect == SkillEffect::SkillEffectInterrupt &&
            interrupts != AB::Entities::SkillTypeAll &&
            !current.CanInterrupt(interrupts))
            // If this is an interrupt skill, and interrupts argument was passed, check also that
            return Iteration::Continue;

        results.push_back(index);
        // Calculate some score, depending on activation time, costs.... Smaller is better
        sorting[index] = current.CalculateCost([this, &current](CostType type)
        {
            switch (type)
            {
            case CostType::Activation:
                return 0.5f;
            case CostType::Recharge:
                return 0.0f;
            case CostType::Energy:
            {
                const float er = resourceComp_->GetEnergyRatio();
                return 1.0f - er;
            }
            case CostType::Adrenaline:
            {
                const int a = resourceComp_->GetAdrenaline();
                return static_cast<float>(a) / static_cast<float>(current.adrenaline_);
            }
            case CostType::HpSacrify:
            {
                const float hr = resourceComp_->GetHealthRatio();
                return 1.0f - hr;
            }
            }
            return 0.0f;
        });
        // Prefer Elite skills
        if (current.data_.isElite)
            sorting[index] = sorting[index] * 0.5f;
        return Iteration::Continue;
    });

    if (results.size() == 0)
        return false;

    ea::sort(results.begin(), results.end(), [&sorting](int i, int j)
    {
        return sorting[i] < sorting[j];
    });
    return true;
}

int Npc::GetBestSkillIndex(SkillEffect effect, SkillEffectTarget target,
    AB::Entities::SkillType interrupts /* = AB::Entities::SkillTypeAll */,
    const Actor* targetActor /* = nullptr */)
{
    ea::vector<int> skillIndices;
    if (GetSkillCandidates(skillIndices, effect, target, interrupts, targetActor))
        return *skillIndices.begin();
    return -1;
}

bool Npc::IsWander() const
{
    return !!wanderComp_;
}

void Npc::SetWander(bool value)
{
    if (!value)
    {
        if (wanderComp_)
            wanderComp_.reset();
        return;
    }

    if (!wanderComp_)
        wanderComp_ = ea::make_unique<Components::WanderComp>(*this);
}

void Npc::WriteSpawnData(Net::NetworkMessage& msg)
{
    if (!serverOnly_)
        Actor::WriteSpawnData(msg);
}

void Npc::Say(ChatType channel, const std::string& message)
{
    switch (channel)
    {
    case ChatType::Map:
    {
        ea::shared_ptr<ChatChannel> ch = GetSubsystem<Chat>()->Get(ChatType::Map, static_cast<uint64_t>(GetGame()->id_));
        if (ch)
            ch->TalkNpc(*this, message);
        break;
    }
    case ChatType::Party:
    {
        ea::shared_ptr<ChatChannel> ch = GetSubsystem<Chat>()->Get(ChatType::Party, GetGroupId());
        if (ch)
            ch->TalkNpc(*this, message);
        break;
    }
    default:
        // N/A
        break;
    }
}

std::string Npc::GetQuote(int index)
{
    if (!HaveFunction(FunctionOnGetQuote))
        return "";
    const char* q = static_cast<const char*>(luaState_["onGetQuote"](index));
    return q;
}

void Npc::_LuaSetName(const std::string& name)
{
    name_ = name;
}

void Npc::_LuaAddWanderPoint(const Math::StdVector3& point)
{
    if (IsWander())
        wanderComp_->AddRoutePoint(point);
}

void Npc::_LuaAddWanderPoints(const std::vector<Math::StdVector3>& points)
{
    for (const auto& point : points)
        _LuaAddWanderPoint(point);
}

bool Npc::SayQuote(ChatType channel, int index)
{
    std::string quote = GetQuote(index);
    if (quote.empty())
        return false;

    kainjow::mustache::mustache tpl{ quote };
    kainjow::mustache::data data;
    if (auto sel = GetSelectedObject())
        data.set("selected_name", sel->GetName());
    else
        data.set("selected_name", "");

    std::string t = tpl.render(data);

    Say(channel, t);
    return true;
}

void Npc::ShootAt(const std::string& itemUuid, Actor* target)
{
    auto game = GetGame();
    game->AddProjectile(itemUuid, GetPtr<Actor>(), target->GetPtr<Actor>());
}

void Npc::OnSelected(Actor* selector)
{
    if (luaInitialized_ && selector)
        Lua::CallFunction(luaState_, "onSelected", selector);
}

void Npc::OnClicked(Actor* selector)
{
    if (luaInitialized_ && selector)
        Lua::CallFunction(luaState_, "onClicked", selector);
    if (Is<Player>(selector))
    {
        if (!IsInRange(Ranges::Adjecent, selector))
            return;
        // Get quests for this player
        auto& player = To<Player>(*selector);
        const auto quests = GetQuestsForPlayer(player);
        player.TriggerQuestSelectionDialog(id_, quests);
    }
}

void Npc::OnArrived()
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onArrived");
}

void Npc::OnCollide(GameObject* other)
{
    if (luaInitialized_ && other)
        Lua::CallFunction(luaState_, "onCollide", other);
}

void Npc::OnTrigger(GameObject* other)
{
    if (luaInitialized_ && HaveFunction(FunctionOnTrigger))
        Lua::CallFunction(luaState_, "onTrigger", other);
}

void Npc::OnLeftArea(GameObject* other)
{
    if (luaInitialized_ && HaveFunction(FunctionOnLeftArea))
        Lua::CallFunction(luaState_, "onLeftArea", other);
}

void Npc::OnEndUseSkill(Skill* skill)
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onEndUseSkill", skill);
}

void Npc::OnStartUseSkill(Skill* skill)
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onStartUseSkill", skill);
}

void Npc::OnAttack(Actor* target, bool& canAttack)
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onAttack", target, canAttack);
}

void Npc::OnAttacked(Actor* source, DamageType type, int32_t damage, bool& canGetAttacked)
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onAttacked", source, type, damage, canGetAttacked);
}

void Npc::OnGettingAttacked(Actor* source, bool& canGetAttacked)
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onGettingAttacked", source, canGetAttacked);
}

void Npc::OnUseSkill(Actor* target, Skill* skill, bool& success)
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onUseSkill", target, skill, success);
}

void Npc::OnSkillTargeted(Actor* source, Skill* skill, bool& success)
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onSkillTargeted", source, skill, success);
}

void Npc::OnInterruptingAttack(bool& success)
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onInterruptingAttack", success);
}

void Npc::OnInterruptingSkill(AB::Entities::SkillType type, Skill* skill, bool& success)
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onInterruptingSkill", type, skill, success);
}

void Npc::OnInterruptedAttack()
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onInterruptedAttack");
}

void Npc::OnInterruptedSkill(Skill* skill)
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onInterruptedSkill", skill);
}

void Npc::OnKnockedDown(uint32_t time)
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onKnockedDown", time);
}

void Npc::OnHealed(int hp)
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onHealed", hp);
}

void Npc::OnDied(Actor*, Actor*)
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onDied");
}

void Npc::OnResurrected(int, int)
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onResurrected");
}

void Npc::_LuaAddQuest(uint32_t index)
{
    quests_.emplace(index);
}

ea::set<uint32_t> Npc::GetQuestsForPlayer(const Player& player) const
{
    ea::set<uint32_t> result;
    const auto& qc = *player.questComp_;
    for (auto qIndex : quests_)
    {
        if (qc.IsAvailable(qIndex))
            result.emplace(qIndex);
    }
    return result;
}

bool Npc::HaveQuestsForPlayer(const Player& player) const
{
    const auto& qc = *player.questComp_;
    for (auto qIndex : quests_)
    {
        if (qc.IsAvailable(qIndex))
            return true;
    }
    return false;
}

}
