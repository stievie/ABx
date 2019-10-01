#pragma once

#include <AB/Entities/MailList.h>
#include <AB/Entities/Mail.h>

namespace IO {

bool IOMail_LoadMailList(AB::Entities::MailList& ml, const std::string& accountUuid);
bool IOMail_SendMailToPlayer(const std::string& playerName, const std::string& fromAcc,
    const std::string& fromName,
    const std::string& subject, const std::string& message);
bool IOMail_SendMailToAccount(const std::string& accountUuid, const std::string& fromAcc,
    const std::string& fromName, const std::string& toName,
    const std::string& subject, const std::string& message);
bool IOMail_ReadMail(AB::Entities::Mail& mail);
bool IOMail_DeleteMail(AB::Entities::Mail& mail);

}
