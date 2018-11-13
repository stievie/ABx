#include "stdafx.h"
#include "Npc.h"
#include "GameManager.h"
#include "ScriptManager.h"
#include "MathUtils.h"
#include "DataProvider.h"
#include "Subsystems.h"

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
        .addFunction("Say", &Npc::Say)
    );
}

Npc::Npc() :
    Actor(),
    luaInitialized_(false),
    aiCharacter_(nullptr)
{
    InitializeLua();
}

Npc::~Npc()
{
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
            LOG_WARNING << "Unable to read primary profession, index = " << skills_.prof1_.index << std::endl;
        }
    }
    skills_.prof2_.index = luaState_["prof1Index"];
    if (skills_.prof2_.index != 0)
    {
        if (!client->Read(skills_.prof2_))
        {
            LOG_WARNING << "Unable to read secondary profession, index = " << skills_.prof2_.index << std::endl;
        }
    }

    behaviours_ = "/scripts/actors/npcs/behaviours.lua";

    bool ret = luaState_["onInit"]();
    if (ret)
    {
        if (!behaviours_.empty())
        {
            IO::AiLoader loader;
            auto dp = GetSubsystem<IO::DataProvider>();
            std::string btFile = dp->GetDataFile(behaviours_);
            if (loader.init(btFile))
            {
                std::vector<std::string> trees;
                loader.getTrees(trees);
                auto randomIter = ai::randomElement(trees.begin(), trees.end());
//                const ai::TreeNodePtr& root = loader.load(*randomIter);
                const ai::TreeNodePtr& root = loader.load("wander");
                ai_ = std::make_shared<ai::AI>(root);
            }
            else
                LOG_ERROR << "could not load the tree: " << loader.getError();
        }

    }
    return ret;
}

std::shared_ptr<ai::AI> Npc::GetAi()
{
    if (!aiCharacter_)
    {
        aiCharacter_ = std::make_shared<AiCharacter>(*this, GetGame()->map_.get());
        ai_->setCharacter(aiCharacter_);
    }
    return ai_;
}

void Npc::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    Actor::Update(timeElapsed, message);
    if (luaInitialized_ && ScriptManager::FunctionExists(luaState_, "onUpdate"))
        luaState_["onUpdate"](timeElapsed);
    if (aiCharacter_)
    {
        const Math::Vector3& pos = transformation_.position_;
        aiCharacter_->setPosition(glm::vec3(pos.x_, pos.y_, pos.z_));
        aiCharacter_->setOrientation(transformation_.rotation_);
        aiCharacter_->setSpeed(GetSpeed());
        aiCharacter_->update(timeElapsed, false);
    }
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
    if (luaInitialized_ && ScriptManager::FunctionExists(luaState_, "onSelected"))
        luaState_["onSelected"](selector);
}

void Npc::OnClicked(std::shared_ptr<Actor> selector)
{
    Actor::OnSelected(selector);
    if (luaInitialized_ && ScriptManager::FunctionExists(luaState_, "onClicked"))
        luaState_["onClicked"](selector);
}

void Npc::OnArrived()
{
    Actor::OnArrived();
    if (luaInitialized_ && ScriptManager::FunctionExists(luaState_, "onArrived"))
        luaState_["onArrived"]();
}

void Npc::OnCollide(std::shared_ptr<Actor> other)
{
    Actor::OnCollide(other);

    if (luaInitialized_ && ScriptManager::FunctionExists(luaState_, "onCollide"))
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
        if (luaInitialized_ && ScriptManager::FunctionExists(luaState_, "onTrigger"))
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

AiCharacter::AiCharacter(Npc& owner, const Map* map) :
    ai::ICharacter(owner.id_),
    owner_(owner),
    map_(map)
{
    const Math::Vector3& pos = owner.transformation_.position_;
    setPosition(glm::vec3(pos.x_, pos.y_, pos.z_));
    setOrientation(owner.transformation_.rotation_);
    setSpeed(owner.GetSpeed());
}

void AiCharacter::update(int64_t, bool)
{
    const float sizeF = static_cast<float>(map_->GetSize());
    const glm::vec3& currentPos = _position;
    glm::vec3 newPos(currentPos);
    if (currentPos.x < -sizeF) {
        newPos.x = sizeF;
    }
    else if (currentPos.x > sizeF) {
        newPos.x = -sizeF;
    }
    if (currentPos.z < -sizeF) {
        newPos.z = sizeF;
    }
    else if (currentPos.z > sizeF) {
        newPos.z = -sizeF;
    }
    setPosition(newPos);
}

void AiCharacter::setPosition(const glm::vec3& position)
{
    ai::ICharacter::setPosition(position);
    owner_.autorunComp_.Goto(Math::Vector3(position.x, position.y, position.z));
}

void AiCharacter::setOrientation(float orientation)
{
    ai::ICharacter::setOrientation(-orientation);
    owner_.transformation_.rotation_ = orientation;
}

void AiCharacter::setSpeed(float speed)
{
    ai::ICharacter::setSpeed(speed);
    owner_.SetSpeed(speed);
}

}
