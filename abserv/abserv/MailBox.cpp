#include "stdafx.h"
#include "MailBox.h"
#include "IOMail.h"

namespace Game {

void MailBox::Update()
{
    IO::IOMail_LoadMailList(mailList_, accountUuid_);
}

bool MailBox::ReadMail(const std::string& uuid, AB::Entities::Mail& mail)
{
    mail.uuid = uuid;
    return IO::IOMail_ReadMail(mail);
}

bool MailBox::DeleteMail(const std::string& uuid, AB::Entities::Mail& mail)
{
    mail.uuid = uuid;
    return IO::IOMail_DeleteMail(mail);
}

void MailBox::DeleteAll()
{
    for (const AB::Entities::MailHeader& mh : mailList_.mails)
    {
        AB::Entities::Mail m;
        m.uuid = mh.uuid;
        IO::IOMail_DeleteMail(m);
    }
}

}
