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

#include <unordered_map>
#include <memory>
#include <abscommon/Variant.h>

namespace HTTP {

struct Session
{
    static uint32_t sessionLifetime_;
    time_t expires_;
    Utils::VariantMap values_;

    Session()
    {
        Touch();
    }

    void Touch()
    {
        time(&expires_);
        struct tm* tm = localtime(&expires_);
        tm->tm_sec += sessionLifetime_ / 1000;
        expires_ = mktime(tm);
    }
};

class Sessions
{
private:
    std::unordered_map<std::string, std::shared_ptr<Session>> sessions_;
public:
    Sessions() = default;
    ~Sessions() = default;

    void Add(const std::string& id, std::shared_ptr<Session> session);
    std::shared_ptr<Session> Get(const std::string& id);
};

}
