#pragma once

#include "GameObject.h"
#include "Script.h"

namespace Game {

class Actor;

class AreaOfEffect : public GameObject
{
private:
    enum Function : uint32_t
    {
        FunctionNone = 0,
        FunctionUpdate = 1,
        FunctionEnded = 1 << 1,
        FunctionOnTrigger = 1 << 2,
        FunctionOnLeftArea = 1 << 3,
        FunctionOnCollide = 1 << 4,
    };
    std::weak_ptr<Actor> source_;
    kaguya::State luaState_;
    bool luaInitialized_;
    std::shared_ptr<Script> script_;
    /// Effect or skill index
    uint32_t index_{ 0 };
    Ranges range_{ Ranges::Aggro };
    uint32_t functions_{ FunctionNone };
    int64_t startTime_;
    // Lifetime
    uint32_t lifetime_;
    uint32_t itemIndex_{ 0 };
    bool HaveFunction(Function func) const
    {
        return (functions_ & func) == func;
    }
    void InitializeLua();
    void _LuaSetSource(Actor* source);
    Actor* _LuaGetSource();
public:
    static void RegisterLua(kaguya::State& state);

    AreaOfEffect();
    // non-copyable
    AreaOfEffect(const AreaOfEffect&) = delete;
    AreaOfEffect& operator=(const AreaOfEffect&) = delete;

    bool LoadScript(const std::string& fileName);
    AB::GameProtocol::GameObjectType GetType() const final override
    {
        return AB::GameProtocol::ObjectTypeAreaOfEffect;
    }
    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;
    void OnCollide(GameObject* other) override;
    void OnTrigger(GameObject* other) override;
    void OnLeftArea(GameObject* other) override;

    void SetRange(Ranges range);
    Ranges GetRange() const { return range_; }
    void SetSource(std::shared_ptr<Actor> source);
    std::shared_ptr<Actor> GetSource();
    uint32_t GetLifetime() const { return lifetime_; }
    void SetLifetime(uint32_t value) { lifetime_ = value; }
    int64_t GetStartTime() const { return startTime_; }
    uint32_t GetIndex() const { return index_; }
    void SetIndex(uint32_t value) { index_ = value; }

    uint32_t GetGroupId() const;
    uint32_t GetItemIndex() const;
    void SetItemIndex(uint32_t value) { itemIndex_ = value; }

    bool Serialize(IO::PropWriteStream& stream) override;
    void WriteSpawnData(Net::NetworkMessage& msg) override;
};

}
