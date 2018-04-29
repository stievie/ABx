#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

constexpr auto KEY_MAILLIST = "mail_list";

/// List of mails. UUID is the receiver account UUID.
struct MailList : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_MAILLIST;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.container(mailUuids, Limits::MAX_MAIL_COUNT, [&s](std::string& m)
        {
            s.text1b(m, Limits::MAX_UUID);
        });
    }
    
    std::vector<std::string> mailUuids;
};

}
}
