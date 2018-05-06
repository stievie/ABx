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
    if (IO::IOMail::LoadMailList(mailList_, accountUuid_))
    {
        newMail_ = static_cast<int>(mailList_.mails.size()) - oldMailCount_;
        // Needed because user will get notified until PLAYER_CHECK_MAIL_MS passed
        if (newMail_ > 0 && lastCheck_ != 0)
            notifiedNewMail_ = false;
        oldMailCount_ = static_cast<int>(mailList_.mails.size());
    }
    lastCheck_ = tick;
}

bool MailBox::ReadMail(const std::string& uuid, AB::Entities::Mail& mail)
{
    mail.uuid = uuid;
    bool ret = IO::IOMail::ReadMail(mail);
    // Get updated mail list
    lastCheck_ = 0;
    return ret;
}

bool MailBox::DeleteMail(const std::string& uuid, AB::Entities::Mail& mail)
{
    mail.uuid = uuid;
    bool ret = IO::IOMail::DeleteMail(mail);
    // Get updated mail list
    lastCheck_ = 0;
    return ret;
}

}
