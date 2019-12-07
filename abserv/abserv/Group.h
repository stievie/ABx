#pragma once

#include <vector>
#include <memory>
#include <sa/Iteration.h>
#include <sa/IdGenerator.h>
#include <kaguya/kaguya.hpp>
#include "Mechanic.h"

namespace Game {

class Actor;

enum class TeamColor
{
    Default = 0,
    Red,
    Blue,
    Yellow
};

class Group
{
private:
    bool _LuaAdd(Actor* actor);
    bool _LuaRemove(Actor* actor);
    Actor* _LuaGetMember(int index);
    int _LuaGetMemberCount();
protected:
    std::vector<std::weak_ptr<Actor>> members_;
    TeamColor color_{ TeamColor::Default };
    uint32_t id_;
    std::string name_;
public:
    static sa::IdGenerator<uint32_t> groupIds_;
    /// Returns a new Party/Group ID
    static uint32_t GetNewId()
    {
        return groupIds_.Next();
    }
    static void RegisterLua(kaguya::State& state);

    explicit Group(uint32_t id);
    Group(uint32_t id, TeamColor color);

    TeamColor GetColor() const { return color_; }
    void SetColor(TeamColor value) { color_ = value; }
    const std::string& GetName() const { return name_; }
    void SetName(const std::string value) { name_ = value; }
    uint32_t GetId() const { return id_; }
    bool IsEnemy(const Group* other) const;
    bool IsAlly(const Group* other) const;
    int GetMorale() const;

    bool Add(std::shared_ptr<Actor> actor);
    bool Remove(uint32_t id);
    Actor* GetLeader() const;

    Actor* GetRandomMember() const;
    Actor* GetRandomMemberInRange(const Actor* actor, Ranges range) const;

    template <typename Callback>
    void VisitMembers(const Callback callback) const
    {
        for (auto& m : members_)
        {
            if (auto sm = m.lock())
            {
                if (callback(*sm) != Iteration::Continue)
                    break;
            }
        }
    }

};

}
