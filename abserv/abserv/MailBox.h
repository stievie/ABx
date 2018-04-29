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
public:
    MailBox(const std::string& accountUuid) :
        accountUuid_(accountUuid),
        oldMailCount_(0),
        newMail_(0)
    {}
    ~MailBox();

    void Update();
    int GetNewMailCount() const
    {
        return newMail_;
    }
    int GetTotalMailCount() const
    {
        return static_cast<int>(mailList_.mailUuids.size());
    }

};

}
