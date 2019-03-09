#pragma once

#include "Actor.h"
#include "Chat.h"
#include "Script.h"
#include "AiLoader.h"
#include "AiCharacter.h"
#include "TriggerComp.h"

namespace Game {

class Npc;
class Map;

class Npc final : public Actor
{
private:
    enum Function : uint32_t
    {
        FunctionNone = 0,
        FunctionUpdate = 1,
        FunctionOnTrigger = 1 << 1,
    };

    friend class AI::AiCharacter;
    /// This NPC exists only on the server, i.e. is not spawned on the client, e.g. a trigger box.
    bool serverOnly_;
    std::string name_;
    uint32_t level_;
    uint32_t modelIndex_;
    AB::Entities::CharacterSex sex_;
    /// Group is like a party and they need unique IDs.
    /// If this NPC belongs to a party with players this must be the PartyID.
    uint32_t groupId_;
    std::string behaviorTree_;
    std::shared_ptr<Script> script_;
    std::shared_ptr<AI::AiCharacter> aiCharacter_;
    std::shared_ptr<ai::AI> ai_;
    uint32_t functions_;
    bool HaveFunction(Function func)
    {
        return (functions_ & func) == func;
    }
protected:
    kaguya::State luaState_;
    bool luaInitialized_;
    void InitializeLua();
    void OnArrived() override;
public:
    static void RegisterLua(kaguya::State& state);

    Npc();
    ~Npc() override;
    // non-copyable
    Npc(const Npc&) = delete;
    Npc& operator=(const Npc&) = delete;

    bool LoadScript(const std::string& fileName);
    AB::GameProtocol::GameObjectType GetType() const override
    {
        return AB::GameProtocol::ObjectTypeNpc;
    }

    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;
    bool Die() override;
    bool Resurrect(int precentHealth, int percentEnergy) override;
    std::shared_ptr<ai::AI> GetAi();
    void Shutdown();

    std::string GetName() const override { return name_; }
    void SetName(const std::string& name) { name_ = name; }
    uint32_t GetLevel() const override { return level_; }
    uint32_t GetModelIndex() const final override
    {
        return modelIndex_;
    }
    AB::Entities::CharacterSex GetSex() const final override
    {
        return sex_;
    }
    uint32_t GetGroupId() const final override
    {
        return groupId_;
    }
    void SetGroupId(uint32_t value);
    bool SetBehaviour(const std::string& name);
    const std::string& GetBehaviour() const { return behaviorTree_; }
    float GetAggro(Actor* other);
    bool IsServerOnly() const { return serverOnly_; }
    void SetServerOnly(bool value) { serverOnly_ = value; }
    uint32_t GetRetriggerTimout() const
    {
        if (!triggerComp_)
            return 0;
        return triggerComp_->retriggerTimeout_;
    }
    void SetRetriggerTimout(uint32_t value)
    {
        if (!triggerComp_)
            triggerComp_ = std::make_unique<Components::TriggerComp>(*this);
        triggerComp_->retriggerTimeout_ = value;
    }
    bool IsTrigger() const
    {
        return triggerComp_ && triggerComp_->trigger_;
    }
    void SetTrigger(bool value)
    {
        if (!triggerComp_)
            triggerComp_ = std::make_unique<Components::TriggerComp>(*this);
        triggerComp_->trigger_ = value;
    }
    void WriteSpawnData(Net::NetworkMessage& msg) override;

    void Say(ChatType channel, const std::string& message);

    void OnSelected(Actor* selector) override;
    void OnClicked(Actor* selector) override;
    void OnCollide(Actor* other) override;
    void OnTrigger(Actor* other) override;
    void OnEndUseSkill(Skill* skill) override;
    void OnStartUseSkill(Skill* skill) override;

    std::unique_ptr<Components::TriggerComp> triggerComp_;
};

}
