#pragma once

#include "Creature.h"
#include "Chat.h"

namespace Game {

class Npc final : public Creature
{
private:
    std::string name_;
    uint32_t level_;
    uint32_t modelIndex_;
    AB::Entities::CharacterSex sex_;
protected:
    kaguya::State luaState_;
    bool luaInitialized_;
    void InitializeLua();
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

    std::string GetName() const override { return name_; }
    uint32_t GetLevel() const override { return level_; }
    uint32_t GetModelIndex() const final override
    {
        return modelIndex_;
    }
    AB::Entities::CharacterSex GetSex() const final override
    {
        return sex_;
    }

    void Say(ChatType channel, const std::string& message);

    void OnSelected(std::shared_ptr<Creature> selector) override;
    void OnClicked(std::shared_ptr<Creature> selector) override;
    void OnCollide(std::shared_ptr<Creature> other) override;
    void OnTrigger(std::shared_ptr<Creature> other) override;
};

}
