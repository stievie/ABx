#include "stdafx.h"
#include "Npc.h"
#include "GameManager.h"
#include "ScriptManager.h"
#include "MathUtils.h"
#include "DataProvider.h"
#include "Subsystems.h"
#include "Random.h"
#include "Party.h"
#include <Mustache/mustache.hpp>
#include "BevaviorCache.h"

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

        .addFunction("SetLevel", &Npc::SetLevel)             // Can only be used in onInit(), i.e. before it is sent to the clients
        .addFunction("Say", &Npc::Say)
        .addFunction("SayQuote", &Npc::SayQuote)
        .addFunction("ShootAt", &Npc::ShootAt)
        .addFunction("GetGroupId", &Npc::GetGroupId)
        .addFunction("SetGroupId", &Npc::SetGroupId)
        .addFunction("SetBehavior", &Npc::SetBehavior)
        .addFunction("IsWander", &Npc::IsWander)
        .addFunction("SetWander", &Npc::SetWander)
        .addFunction("AddWanderPoint", &Npc::_LuaAddWanderPoint)
        .addFunction("AddWanderPoints", &Npc::_LuaAddWanderPoints)
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
    events_.Subscribe<void(void)>(EVENT_ON_DIED, std::bind(&Npc::OnDied, this));
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
    groupId_ = Party::GetNewId();
    InitializeLua();
}

Npc::~Npc() = default;

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
        stateComp_.SetState(AB::GameProtocol::CreatureStateIdle, true);

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
        functions_ |= FunctionUpdate;
    if (Lua::IsFunction(luaState_, "onTrigger"))
        functions_ |= FunctionOnTrigger;
    if (Lua::IsFunction(luaState_, "onLeftArea"))
        functions_ |= FunctionOnLeftArea;
    if (Lua::IsFunction(luaState_, "onGetQuote"))
        functions_ |= FunctionOnGetQuote;

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

void Npc::SetGroupId(uint32_t value)
{
    if (groupId_ != value)
    {
        groupId_ = value;
    }
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
    aiComp_ = std::make_unique<Components::AiComp>(*this);
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
    const float rval = random->GetFloat();
    return (dist + health) * rval;
}

int Npc::GetBestSkillIndex(SkillEffect effect, SkillTarget target,
    AB::Entities::SkillType interrupts /* = AB::Entities::SkillTypeAll */,
    const Actor* targetActor /* = nullptr */)
{
    // skill index -> cost (smaller is better)
    std::map<int, float> sorting;
    std::vector<int> skillIndices;
    skills_->VisitSkills([&](int index, const Skill& current)
    {
        // If there is a target check if it's in range
        if (targetActor && !IsInRange(current.GetRange(), targetActor))
            return Iteration::Continue;
        if (!current.HasEffect(effect))
            return Iteration::Continue;
        if (!current.HasTarget(target))
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

        skillIndices.push_back(index);
        // Calculate some score, depending on activation time, costs...
        sorting[index] = current.CalculateCost([this, &current](CostType type)
        {
            switch (type)
            {
            case CostType::Activation:
                // Activation always plays a role
                return 1.0f;
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
        return Iteration::Continue;
    });

    if (skillIndices.size() == 0)
        return -1;

    std::sort(skillIndices.begin(), skillIndices.end(), [&sorting](int i, int j)
    {
        return sorting[i] < sorting[j];
    });

    return *skillIndices.begin();
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
        wanderComp_ = std::make_unique<Components::WanderComp>(*this);
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
        std::shared_ptr<ChatChannel> ch = GetSubsystem<Chat>()->Get(ChatType::Map, static_cast<uint64_t>(GetGame()->id_));
        if (ch)
            ch->TalkNpc(*this, message);
        break;
    }
    case ChatType::Party:
    {
        std::shared_ptr<ChatChannel> ch = GetSubsystem<Chat>()->Get(ChatType::Party, GetGroupId());
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

void Npc::_LuaAddWanderPoint(const Math::STLVector3& point)
{
    if (IsWander())
        wanderComp_->AddRoutePoint(point);
}

void Npc::_LuaAddWanderPoints(const std::vector<Math::STLVector3>& points)
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
    if (auto sel = selectedObject_.lock())
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
    game->AddProjectile(itemUuid, GetThis<Actor>(), target->GetThis<Actor>());
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

void Npc::OnDied()
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onDied");
}

void Npc::OnResurrected(int, int)
{
    if (luaInitialized_)
        Lua::CallFunction(luaState_, "onResurrected");
}

}
