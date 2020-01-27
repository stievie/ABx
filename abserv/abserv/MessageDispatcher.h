/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

namespace Net {
class MessageMsg;
}

class MessageDispatcher
{
private:
    void DispatchGuildChat(const Net::MessageMsg& msg);
    void DispatchTradeChat(const Net::MessageMsg& msg);
    void DispatchWhipserChat(const Net::MessageMsg& msg);
    void DispatchNewMail(const Net::MessageMsg& msg);
    void DispatchPlayerChanged(const Net::MessageMsg& msg);
    void DispatchServerChange(const Net::MessageMsg& msg);
    void DispatchTeamsEnterMatch(const Net::MessageMsg& msg);
    void DispatchQueueAdded(const Net::MessageMsg& msg);
    void DispatchQueueRemoved(const Net::MessageMsg& msg);
public:
    MessageDispatcher() = default;
    ~MessageDispatcher() = default;

    void Dispatch(const Net::MessageMsg& msg);
};

