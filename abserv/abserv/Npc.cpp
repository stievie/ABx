#include "stdafx.h"
#include "Npc.h"
#include "GameManager.h"
#include "ScriptManager.h"
#include "MathUtils.h"
#include "DataProvider.h"
#include "Subsystems.h"
#include "AiCharacter.h"
#include "Random.h"

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
    );
}

Npc::Npc() :
    Actor(),
    behaviorTree_(""),
    luaInitialized_(false),
    aiCharacter_(nullptr)
{
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
    stateComp_.SetState(luaState_["creatureState"], true);

    IO::DataClient* client = GetSubsystem<IO::DataClient>();

    skills_.prof1_.index = luaState_["prof1Index"];
    if (skills_.prof1_.index != 0)
    {
        if (!client->Read(skills_.prof1_))
        {
            LOG_WARNING << "Unable to read primary profession of " << GetName() << ", index = " << skills_.prof1_.index << std::endl;
        }
    }
    if (ScriptManager::IsNumber(luaState_, "prof2Index"))
    {
        skills_.prof2_.index = luaState_["prof2Index"];
        if (skills_.prof2_.index != 0)
        {
            if (!client->Read(skills_.prof2_))
            {
                LOG_WARNING << "Unable to read secondary profession of " << GetName() << ", index = " << skills_.prof2_.index << std::endl;
            }
        }
    }

    if (ScriptManager::IsString(luaState_, "behavior"))
        behaviorTree_ = (const char*)luaState_["behavior"];

    bool ret = luaState_["onInit"]();
    if (ret)
    {
        if (!behaviorTree_.empty())
        {
            auto loader = GetSubsystem<AI::AiLoader>();
            std::vector<std::string> trees;
            loader->GetTrees(trees);
            const ai::TreeNodePtr& root = loader->Load(behaviorTree_);
            if (root)
                ai_ = std::make_shared<ai::AI>(root);
        }
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
    if (luaInitialized_ && ScriptManager::IsFunction(luaState_, "onUpdate"))
        luaState_["onUpdate"](timeElapsed);
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
            GetGame()->map_->AddEntity(GetAi(), 0);
        }
    }
    return true;
}

float Npc::GetAggro(Actor* other)
{
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
        {
            ch->TalkNpc(this, message);
        }
        break;
    }
    case ChatType::Allies:
        break;
    case ChatType::Party:
        break;
    }
}

void Npc::OnSelected(std::shared_ptr<Actor> selector)
{
    Actor::OnSelected(selector);
    if (luaInitialized_ && ScriptManager::IsFunction(luaState_, "onSelected"))
        luaState_["onSelected"](selector);
}

void Npc::OnClicked(std::shared_ptr<Actor> selector)
{
    Actor::OnSelected(selector);
    if (luaInitialized_ && ScriptManager::IsFunction(luaState_, "onClicked"))
        luaState_["onClicked"](selector);
}

void Npc::OnArrived()
{
    Actor::OnArrived();
    if (luaInitialized_ && ScriptManager::IsFunction(luaState_, "onArrived"))
        luaState_["onArrived"]();
}

void Npc::OnCollide(std::shared_ptr<Actor> other)
{
    Actor::OnCollide(other);

    if (luaInitialized_ && ScriptManager::IsFunction(luaState_, "onCollide"))
        luaState_["onCollide"](other);

    if (trigger_)
        OnTrigger(other);
}

void Npc::OnTrigger(std::shared_ptr<Actor> other)
{
    Actor::OnTrigger(other);
    int64_t tick = Utils::AbTick();
    int64_t lasTrigger = triggered_[other->id_];
    if (static_cast<uint32_t>(tick - lasTrigger) > retriggerTimeout_)
    {
        if (luaInitialized_ && ScriptManager::IsFunction(luaState_, "onTrigger"))
            luaState_["onTrigger"](other);
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

}
