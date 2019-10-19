#pragma once

#include <AB/Entities/MailList.h>
#include <AB/Entities/Mail.h>

namespace IO {
namespace IOMail {

bool LoadMailList(AB::Entities::MailList& ml, const std::string& accountUuid);
bool SendMailToPlayer(const std::string& playerName, const std::string& fromAcc,
    const std::string& fromName,
    const std::string& subject, const std::string& message);
bool SendMailToAccount(const std::string& accountUuid, const std::string& fromAcc,
    const std::string& fromName, const std::string& toName,
    const std::string& subject, const std::string& message);
bool ReadMail(AB::Entities::Mail& mail);
bool DeleteMail(AB::Entities::Mail& mail);

}
}
