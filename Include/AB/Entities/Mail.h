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

namespace AB {
namespace Entities {

struct Mail : Entity
{
    MAKE_ENTITY(Mail)
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(fromAccountUuid, Limits::MAX_UUID);
        s.text1b(toAccountUuid, Limits::MAX_UUID);
        s.text1b(fromName, Limits::MAX_CHARACTER_NAME);
        s.text1b(toName, Limits::MAX_CHARACTER_NAME);
        s.text1b(subject, Limits::MAX_MAIL_SUBJECT);
        s.text1b(message, Limits::MAX_MAIL_MESSAGE);
        s.value8b(created);
        s.value1b(isRead);
    }

    std::string fromAccountUuid = EMPTY_GUID;
    std::string toAccountUuid = EMPTY_GUID;
    std::string fromName;
    std::string toName;
    std::string subject;
    std::string message;
    timestamp_t created = 0;
    bool isRead = false;
};

}
}
