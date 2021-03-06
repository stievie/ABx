/**
 * Copyright 2020 Stefan Ascher
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

#include "Source.h"
#include <memory>
#include <sa/Compiler.h>
PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_MSVC(4459 4706 4267)
#include <httplib.h>
PRAGMA_WARNING_POP

namespace Sync {

class HttpSource final : public Source
{
private:
    std::string host_;
    uint16_t port_;
    std::unique_ptr<httplib::SSLClient> client_;
    httplib::Headers headers_;
    httplib::SSLClient* GetHttpClient();
    std::mutex lock_;
public:
    HttpSource(const std::string& host, uint16_t port, httplib::Headers headers);
    std::vector<char> GetChunk(const std::string& filename, size_t start = 0, size_t length = 0) override;
    std::function<bool(size_t)> onDownloadProgress_;
};

}
