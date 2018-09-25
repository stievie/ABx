#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_MAILLIST = "mail_list";

struct MailHeader
{
    std::string uuid;
    std::string fromName;
    std::string subject;
    int64_t created;
    bool isRead;
};

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
        s.container(mails, Limits::MAX_MAIL_COUNT, [&s](MailHeader& m)
        {
            s.text1b(m.uuid, Limits::MAX_UUID);
            s.text1b(m.fromName, Limits::MAX_CHARACTER_NAME);
            s.text1b(m.subject, Limits::MAX_MAIL_SUBJECT);
            s.value8b(m.created);
            s.value1b(m.isRead);
        });
    }

    std::vector<MailHeader> mails;
};

}
}
