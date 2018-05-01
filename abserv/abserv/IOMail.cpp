#include "stdafx.h"
#include "IOMail.h"
#include "IOPlayer.h"

namespace IO {

bool IOMail::LoadMailList(AB::Entities::MailList& ml, const std::string& accountUuid)
{
    IO::DataClient* client = Application::Instance->GetDataClient();
    ml.uuid = accountUuid;
    return client->Read(ml);
}

bool IOMail::SendMailToPlayer(const std::string& playerName, const std::string& fromAcc,
    const std::string& fromName,
    const std::string& subject, const std::string& message)
{
    AB::Entities::Character ch;
    ch.name = playerName;

    bool ret = IOPlayer::LoadCharacter(ch);
    if (!ret)
        return false;

    return SendMailToAccount(ch.accountUuid, fromAcc, fromName, playerName, subject, message);
}

bool IOMail::SendMailToAccount(const std::string& accountUuid, const std::string& fromAcc,
    const std::string& fromName, const std::string& toName,
    const std::string& subject, const std::string& message)
{
    AB::Entities::Mail m;
    const uuids::uuid guid = uuids::uuid_system_generator{}();
    m.uuid = guid.to_string();
    m.fromAccountUuid = fromAcc;
    m.toAccountUuid = accountUuid;
    m.fromName = fromName;
    m.toName = toName;
    m.subject = subject;
    m.message = message;
    m.created = Utils::AbTick();

    IO::DataClient* client = Application::Instance->GetDataClient();
    bool ret = client->Create(m);
    if (ret)
    {
        // Invalidate recipients mail list
        AB::Entities::MailList ml;
        ml.uuid = accountUuid;
        client->Invalidate(ml);
    }
    return ret;
}

bool IOMail::GetMail(AB::Entities::Mail& mail)
{
    IO::DataClient* client = Application::Instance->GetDataClient();
    bool ret = client->Read(mail);
    if (ret)
    {
        mail.isRead = true;
        client->Update(mail);
        // Invalidate mail list
        AB::Entities::MailList ml;
        ml.uuid = mail.toAccountUuid;
        client->Invalidate(ml);
    }
    return ret;
}

}
