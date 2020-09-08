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

#include "LocalBackend.h"
#include "RemoteBackend.h"
#include <string>
#include <functional>

namespace Sync {

class Synchronizer
{
private:
    LocalBackend& local_;
    RemoteBackend& remote_;
    size_t downloaded_{ 0 };
    size_t copied_{ 0 };
    size_t filesize_{ 0 };
    bool different_{ false };
    void CallProgress(size_t value, size_t max);
public:
    Synchronizer(LocalBackend& local, RemoteBackend& remote);
    bool Synchronize(const std::string& file);

    size_t GetDownloaded() const { return downloaded_; }
    size_t GetCopied() const { return copied_; }
    size_t GetFilesize() const { return filesize_; }
    bool IsDifferent() const { return different_; }

    std::function<void(size_t value, size_t max)> onProgress_;
};

}
