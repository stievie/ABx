#pragma once

#include <AB/Entities/MailList.h>
#include <AB/Entities/Mail.h>

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
        notifiedNewMail_(false),
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
    const AB::Entities::MailList& GetMails() const
    {
        return mailList_;
    }
    bool ReadMail(const std::string& uuid, AB::Entities::Mail& mail);

    bool notifiedFull_;
    bool notifiedNewMail_;
};

}
