#pragma once

#include <vector>
#include <memory>
#include <sa/Iteration.h>
#include <sa/IdGenerator.h>

namespace Game {

class Actor;

enum class TeamColor
{
    NothingSpecial = 0,
    Red,
    Blue,
    Yellow
};

class Group
{
protected:
    std::vector<std::weak_ptr<Actor>> members_;
    TeamColor color_{ TeamColor::NothingSpecial };
    uint32_t id_;
public:
    static sa::IdGenerator<uint32_t> groupIds_;
    /// Returns a new Party/Group ID
    static uint32_t GetNewId()
    {
        return groupIds_.Next();
    }

    explicit Group(uint32_t id);
    Group(uint32_t id, TeamColor color);

    TeamColor GetColor() const { return color_; }
    uint32_t GetId() const { return id_; }

    void Add(std::shared_ptr<Actor> actor);
    void Remove(uint32_t id);
    template <typename Callback>
    void VisistMembers(const Callback callback)
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
