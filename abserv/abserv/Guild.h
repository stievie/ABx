#pragma once

#include <AB/Entities/Guild.h>
#include <AB/Entities/GuildMembers.h>
#include <sa/Iteration.h>

namespace Game {

class Guild
{
public:
    explicit Guild(AB::Entities::Guild&& data);
    ~Guild() = default;
    Guild(const Guild&) = delete;
    Guild& operator=(const Guild&) = delete;

    size_t GetAccounts(std::vector<std::string>& uuids) const;
    bool GetMembers(AB::Entities::GuildMembers& members) const;
    bool IsMember(const std::string& accountUuid) const;
    bool GetMember(const std::string& accountUuid, AB::Entities::GuildMember& member) const;

    template <typename Callback>
    void VisitAll(const Callback& callback) const
    {
        AB::Entities::GuildMembers members;
        if (!GetMembers(members))
            return;

        for (const auto& member : members.members)
            if (callback(member) != Iteration::Continue)
                break;
    }

    AB::Entities::Guild data_;
};

}
