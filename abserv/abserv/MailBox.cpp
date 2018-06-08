#include "stdafx.h"
#include "MailBox.h"
#include "IOMail.h"
#include "Utils.h"

namespace Game {

void MailBox::Update()
{
    IO::IOMail::LoadMailList(mailList_, accountUuid_);
}

bool MailBox::ReadMail(const std::string& uuid, AB::Entities::Mail& mail)
{
    mail.uuid = uuid;
    return IO::IOMail::ReadMail(mail);
}

bool MailBox::DeleteMail(const std::string& uuid, AB::Entities::Mail& mail)
{
    mail.uuid = uuid;
    return IO::IOMail::DeleteMail(mail);
}

}
