#pragma once

#include <sa/CallableTable.h>
#include "MessageBuffer.h"
#include "Message.h"
#include <functional>

namespace IPC {

class MessageHandlers
{
private:
    sa::CallableTable<size_t, void, const MessageBuffer&> handlers_;
public:
    template<typename _Msg, typename _MsgType = typename _Msg::value_type>
    void Add(std::function<void(const _MsgType& msg)>&& func)
    {
        handlers_.Add(_Msg::MessageType, [handler = std::move(func)](const MessageBuffer& buffer)
        {
            auto packet = Get<_Msg::value_type>(buffer);
            handler(packet);
        });
    }
    bool Exists(size_t type) const { return handlers_.Exists(type); }
    void Call(size_t type, const MessageBuffer& buffer)
    {
        handlers_.Call(type, buffer);
    }
};

}
