#pragma once

class MessageDispatcher
{
private:
    void DispatchGuildChat(const Net::MessageMsg& msg);
    void DispatchTradeChat(const Net::MessageMsg& msg);
    void DispatchWhipserChat(const Net::MessageMsg& msg);
    void DispatchNewMail(const Net::MessageMsg& msg);
    void DispatchPlayerLoggedIn(const Net::MessageMsg& msg);
    void DispatchPlayerLoggedOut(const Net::MessageMsg& msg);
    void DispatchServerChange(const Net::MessageMsg& msg);
    void DispatchTeamsEnterMatch(const Net::MessageMsg& msg);
    void DispatchQueueAdded(const Net::MessageMsg& msg);
    void DispatchQueueRemoved(const Net::MessageMsg& msg);
public:
    MessageDispatcher() = default;
    ~MessageDispatcher() = default;

    void Dispatch(const Net::MessageMsg& msg);
};

