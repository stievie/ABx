#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_MAILS = "mails";

struct Mail : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_MAILS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(fromAccountUuid, Limits::MAX_UUID);
        s.text1b(toAccountUuid, Limits::MAX_UUID);
        s.text1b(fromName, Limits::MAX_CHARACTER_NAME);
        s.text1b(toName, Limits::MAX_CHARACTER_NAME);
        s.text1b(subject, Limits::MAX_MAIL_SUBJECT);
        s.text1b(message, Limits::MAX_MAIL_MESSAGE);
        s.value8b(created);
        s.value1b(isRead);
    }

    std::string fromAccountUuid = EMPTY_GUID;
    std::string toAccountUuid = EMPTY_GUID;
    std::string fromName;
    std::string toName;
    std::string subject;
    std::string message;
    int64_t created = 0;
    bool isRead = false;
};

}
}
