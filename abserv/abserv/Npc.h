#pragma once

#include "Actor.h"
#include "Chat.h"
#include "Script.h"

namespace Game {

class Npc final : public Actor
{
private:
    std::string name_;
    uint32_t level_;
    uint32_t modelIndex_;
    AB::Entities::CharacterSex sex_;
    std::shared_ptr<Script> script_;
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

    void Say(ChatType channel, const std::string& message);

    void OnSelected(std::shared_ptr<Actor> selector) override;
    void OnClicked(std::shared_ptr<Actor> selector) override;
    void OnCollide(std::shared_ptr<Actor> other) override;
    void OnTrigger(std::shared_ptr<Actor> other) override;
};

}
