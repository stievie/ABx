#include "stdafx.h"
#include "MailBox.h"
#include "IOMail.h"

namespace Game {

void MailBox::Update()
{
    if (IO::IOMail::LoadMailList(mailList_, accountUuid_))
    {
        // if oldMailCount_ == -1 it was not yet initialized
        if (oldMailCount_ != -1)
            newMail_ = static_cast<int>(mailList_.mails.size()) - oldMailCount_;
        oldMailCount_ = static_cast<int>(mailList_.mails.size());
    }
}

}
