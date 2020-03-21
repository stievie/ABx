/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <vector>
#include <memory>
#include <sa/Iteration.h>
#include <sa/IdGenerator.h>
#include <kaguya/kaguya.hpp>
#include <abshared/Mechanic.h>

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
    std::vector<Actor*> _LuaGetMembers();
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
    void IncreaseMorale();
    void DecreaseMorale();
    void Resurrect(int precentHealth, int percentEnergy);
    void KillAll();

    bool Add(std::shared_ptr<Actor> actor);
    bool Remove(uint32_t id);
    Actor* GetLeader() const;

    Actor* GetRandomMember() const;
    Actor* GetRandomMemberInRange(const Actor* actor, Ranges range) const;

    template <typename Callback>
    void VisitMembers(Callback&& callback) const
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
