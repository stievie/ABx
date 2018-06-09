#pragma once

class MessageDispatcher
{
private:
    void DispatchGuildChat(const Net::MessageMsg& msg);
    void DispatchTradeChat(const Net::MessageMsg& msg);
    void DispatchWhipserChat(const Net::MessageMsg& msg);
    void DispatchNewMail(const Net::MessageMsg& msg);
public:
    MessageDispatcher() = default;
    ~MessageDispatcher() = default;

    void Dispatch(const Net::MessageMsg& msg);
};

