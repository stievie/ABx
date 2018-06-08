#include "stdafx.h"
#include "MailBox.h"
#include "IOMail.h"
#include "Utils.h"

namespace Game {

void MailBox::Update()
{
    if (IO::IOMail::LoadMailList(mailList_, accountUuid_))
    {
        newMail_ = static_cast<int>(mailList_.mails.size()) - oldMailCount_;
        oldMailCount_ = static_cast<int>(mailList_.mails.size());
    }
}

bool MailBox::ReadMail(const std::string& uuid, AB::Entities::Mail& mail)
{
    mail.uuid = uuid;
    bool ret = IO::IOMail::ReadMail(mail);
    return ret;
}

bool MailBox::DeleteMail(const std::string& uuid, AB::Entities::Mail& mail)
{
    mail.uuid = uuid;
    bool ret = IO::IOMail::DeleteMail(mail);
    return ret;
}

}
