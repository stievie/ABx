#pragma once
namespace Game {

struct Mail
{
    uint32_t id;
    uint32_t fromPlayer;
    uint32_t fromAccount;
    uint32_t toPlayer;
    uint32_t toAccount;
    std::string subject;
    std::string message;
    bool isRead;
};

class MailBox
{
private:
    uint32_t playerId_;
    uint32_t accountId_;
public:
    MailBox(uint32_t playerId) :
        playerId_(playerId)
    {}
    ~MailBox();
};

}
