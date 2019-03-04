#include "stdafx.h"
#include "Npc.h"
#include "GameManager.h"
#include "ScriptManager.h"
#include "MathUtils.h"
#include "DataProvider.h"
#include "Subsystems.h"
#include "AiCharacter.h"
#include "Random.h"
#include "Party.h"

namespace Game {

void Npc::InitializeLua()
{
    ScriptManager::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
    luaInitialized_ = true;
}

void Npc::RegisterLua(kaguya::State& state)
{
    state["Npc"].setClass(kaguya::UserdataMetatable<Npc, Actor>()
        .addFunction("SetName", &Npc::SetName)
        .addFunction("SetBehaviour", &Npc::SetBehaviour)
        .addFunction("GetBehaviour", &Npc::GetBehaviour)
        .addFunction("Say", &Npc::Say)
        .addFunction("GetGroupId", &Npc::GetGroupId)
        .addFunction("SetGroupId", &Npc::SetGroupId)
    );
}

Npc::Npc() :
    Actor(),
    behaviorTree_(""),
    luaInitialized_(false),
    aiCharacter_(nullptr)
{
    // Party and Groups must be unique, i.e. share the same ID pool.
    groupId_ = Party::GetNewId();
    InitializeLua();
}

Npc::~Npc()
{
    Shutdown();
}

bool Npc::LoadScript(const std::string& fileName)
{
    script_ = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(fileName);
    if (!script_)
        return false;
    if (!script_->Execute(luaState_))
        return false;

    name_ = (const char*)luaState_["name"];
    level_ = luaState_["level"];
    modelIndex_ = luaState_["modelIndex"];
    sex_ = luaState_["sex"];
    if (ScriptManager::IsNumber(luaState_, "group_id"))
        groupId_ = luaState_["group_id"];

    stateComp_.SetState(luaState_["creatureState"], true);

    IO::DataClient* client = GetSubsystem<IO::DataClient>();

    skills_->prof1_.index = luaState_["prof1Index"];
    if (skills_->prof1_.index != 0)
    {
        if (!client->Read(skills_->prof1_))
        {
            LOG_WARNING << "Unable to read primary profession of " << GetName() << ", index = " << skills_->prof1_.index << std::endl;
        }
    }
    if (ScriptManager::IsNumber(luaState_, "prof2Index"))
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

    if (ScriptManager::IsString(luaState_, "behavior"))
        behaviorTree_ = (const char*)luaState_["behavior"];

    bool ret = luaState_["onInit"]();
    if (ret)
    {
        if (!behaviorTree_.empty())
            SetBehaviour(behaviorTree_);
    }
    return ret;
}

std::shared_ptr<ai::AI> Npc::GetAi()
{
    if (!ai_)
        return std::shared_ptr<ai::AI>();

    if (!aiCharacter_)
    {
        aiCharacter_ = std::make_shared<AI::AiCharacter>(*this, GetGame()->map_.get());
        ai_->setCharacter(aiCharacter_);
    }
    return ai_;
}

void Npc::Shutdown()
{
    if (ai_)
    {
        ai::Zone* zone = ai_->getZone();
        if (zone == nullptr)
            return;
        zone->destroyAI(id_);
        ai_->setZone(nullptr);
    }
}

void Npc::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    Actor::Update(timeElapsed, message);
    if (luaInitialized_)
        ScriptManager::CallFunction(luaState_, "onUpdate", timeElapsed);
}

bool Npc::Die()
{
    bool res = Actor::Die();
    if (res)
    {
        if (luaInitialized_)
            ScriptManager::CallFunction(luaState_, "onDied");
    }
    return res;
}

bool Npc::Resurrect(int precentHealth, int percentEnergy)
{
    bool res = Actor::Resurrect(precentHealth, percentEnergy);
    if (res)
    {
        if (luaInitialized_)
            ScriptManager::CallFunction(luaState_, "onResurrected");
    }
    return res;
}

void Npc::SetGroupId(uint32_t value)
{
    if (groupId_ != value)
    {
        GetGame()->map_->SetEntityGroupId(GetAi(), groupId_, value);
        groupId_ = value;
    }
}

bool Npc::SetBehaviour(const std::string& name)
{
    if (behaviorTree_.compare(name) != 0)
    {
        behaviorTree_ = name;
        auto loader = GetSubsystem<AI::AiLoader>();
        const ai::TreeNodePtr& root = loader->Load(behaviorTree_);
        if (!root)
            return false;
        if (ai_)
            ai_->setBehaviour(root);
        else
        {
            ai_ = std::make_shared<ai::AI>(root);
            ai_->getAggroMgr().setReduceByValue(0.1f);
            GetGame()->map_->AddEntity(GetAi(), 0);
        }
    }
    return true;
}

float Npc::GetAggro(Actor* other)
{
    if (!other)
        return 0.0f;

    auto random = GetSubsystem<Crypto::Random>();
    float dist = GetPosition().Distance(other->GetPosition());
    float rval = random->GetFloat();
    return (1.0f / dist) * rval;
}

void Npc::Say(ChatType channel, const std::string& message)
{
    switch (channel)
    {
    case ChatType::Map:
    {
        std::shared_ptr<ChatChannel> ch = GetSubsystem<Chat>()->Get(ChatType::Map, static_cast<uint64_t>(GetGame()->id_));
        if (ch)
            ch->TalkNpc(this, message);
        break;
    }
    case ChatType::Party:
    {
        std::shared_ptr<ChatChannel> ch = GetSubsystem<Chat>()->Get(ChatType::Party, GetGroupId());
        if (ch)
            ch->TalkNpc(this, message);
        break;
    }
    }
}

void Npc::OnSelected(Actor* selector)
{
    Actor::OnSelected(selector);
    if (luaInitialized_ && selector)
        ScriptManager::CallFunction(luaState_, "onSelected", selector);
}

void Npc::OnClicked(Actor* selector)
{
    Actor::OnSelected(selector);
    if (luaInitialized_ && selector)
        ScriptManager::CallFunction(luaState_, "onClicked", selector);
}

void Npc::OnArrived()
{
    Actor::OnArrived();
    if (luaInitialized_)
        ScriptManager::CallFunction(luaState_, "onArrived");
}

void Npc::OnCollide(Actor* other)
{
    Actor::OnCollide(other);

    if (luaInitialized_ && other)
        ScriptManager::CallFunction(luaState_, "onCollide", other);

    if (trigger_ && other)
        OnTrigger(other);
}

void Npc::OnTrigger(Actor* other)
{
    Actor::OnTrigger(other);
    if (!other)
        return;

    int64_t tick = Utils::Tick();
    int64_t lasTrigger = triggered_[other->id_];
    if (static_cast<uint32_t>(tick - lasTrigger) > retriggerTimeout_)
    {
        if (luaInitialized_)
            ScriptManager::CallFunction(luaState_, "onTrigger", other);
    }
    triggered_[other->id_] = tick;

    // Delete old
    for (auto it = triggered_.begin(); it != triggered_.end(); )
    {
        if (tick - (*it).second > 10000)
            triggered_.erase(it++);
        else
            ++it;
    }
}

void Npc::OnEndUseSkill(Skill* skill)
{
    Actor::OnEndUseSkill(skill);

    if (luaInitialized_)
        ScriptManager::CallFunction(luaState_, "onEndUseSkill", skill);
}

void Npc::OnStartUseSkill(Skill* skill)
{
    Actor::OnStartUseSkill(skill);

    if (luaInitialized_)
        ScriptManager::CallFunction(luaState_, "onStartUseSkill", skill);
}

}
