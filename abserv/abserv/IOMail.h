#pragma once

#include <AB/Entities/MailList.h>
#include <AB/Entities/Mail.h>

namespace IO {

class IOMail
{
public:
    IOMail() = delete;

    static bool LoadMailList(AB::Entities::MailList& ml, const std::string& accountUuid);
    static bool SendMailToPlayer(const std::string& playerName, const std::string& fromAcc,
        const std::string& fromName,
        const std::string& subject, const std::string& message);
    static bool SendMailToAccount(const std::string& accountUuid, const std::string& fromAcc,
        const std::string& fromName, const std::string& toName,
        const std::string& subject, const std::string& message);
    static bool ReadMail(AB::Entities::Mail& mail);
    static bool DeleteMail(AB::Entities::Mail& mail);
};

}
