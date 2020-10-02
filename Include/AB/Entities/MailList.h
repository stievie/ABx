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

#include <AB/Entities/Entity.h>
#include <vector>

namespace AB {
namespace Entities {

struct MailHeader
{
    std::string uuid;
    std::string fromName;
    std::string subject;
    timestamp_t created;
    bool isRead;
};

/// List of mails. UUID is the receiver account UUID.
struct MailList : Entity
{
    MAKE_ENTITY(MailList)
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.container(mails, Limits::MAX_MAIL_COUNT, [&s](MailHeader& m)
        {
            s.text1b(m.uuid, Limits::MAX_UUID);
            s.text1b(m.fromName, Limits::MAX_CHARACTER_NAME);
            s.text1b(m.subject, Limits::MAX_MAIL_SUBJECT);
            s.value8b(m.created);
            s.value1b(m.isRead);
        });
    }

    std::vector<MailHeader> mails;
};

}
}
