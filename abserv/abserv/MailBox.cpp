#include "stdafx.h"
#include "MailBox.h"
#include "IOMail.h"
#include "Utils.h"

namespace Game {

void MailBox::Update()
{
    int64_t tick = Utils::AbTick();
    if (tick - lastCheck_ < PLAYER_CHECK_MAIL_MS)
        return;
    lastCheck_ = tick;
    if (IO::IOMail::LoadMailList(mailList_, accountUuid_))
    {
        // if oldMailCount_ == -1 it was not yet initialized
        if (oldMailCount_ != -1)
            newMail_ = static_cast<int>(mailList_.mails.size()) - oldMailCount_;
        oldMailCount_ = static_cast<int>(mailList_.mails.size());
    }
}

bool MailBox::GetMail(const std::string& uuid, AB::Entities::Mail& mail)
{
    mail.uuid = uuid;
    return IO::IOMail::GetMail(mail);
}

}
