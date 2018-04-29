#include "stdafx.h"
#include "MailBox.h"
#include "IOMail.h"

namespace Game {

void MailBox::Update()
{
    oldMailCount_ = static_cast<int>(mailList_.mailUuids.size());
    AB::Entities::MailList ml;
    if (IO::IOMail::LoadMailList(ml, accountUuid_))
    {
        mailList_ = ml;
        newMail_ = static_cast<int>(mailList_.mailUuids.size()) - oldMailCount_;
    }
}

MailBox::~MailBox()
{
}

}
