#pragma once

#include <AB/Entities/MailList.h>

namespace Game {

class MailBox
{
private:
    std::string accountUuid_;
    AB::Entities::MailList mailList_;
    int oldMailCount_;
    int newMail_;
    int64_t lastCheck_;
public:
    MailBox(const std::string& accountUuid) :
        accountUuid_(accountUuid),
        oldMailCount_(-1),
        newMail_(0),
        notifiedFull_(false),
        lastCheck_(0)
    {}
    ~MailBox() = default;

    void Update();
    int GetNewMailCount() const
    {
        return newMail_;
    }
    int GetTotalMailCount() const
    {
        return static_cast<int>(mailList_.mails.size());
    }

    bool notifiedFull_;
};

}
