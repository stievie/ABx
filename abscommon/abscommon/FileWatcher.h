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

#include <string>
#include <memory>
#include <filesystem>
#include <functional>

namespace fs = std::filesystem;

namespace IO {

class FileWatcher : std::enable_shared_from_this<FileWatcher>
{
private:
    fs::path path_;
    void* userData_{ nullptr };
    std::function<void(const std::string, void*)> onChanged_;
    fs::file_time_type lastTime_{ fs::file_time_type::min() };
    bool enabled_{ false };
public:
    FileWatcher(const std::string& fileName, void* userData, std::function<void(const std::string, void*)>&& onChanged) :
        path_(fileName),
        userData_(userData),
        onChanged_(std::move(onChanged))
    { }
    ~FileWatcher();
    void Update();
    void Start();
    void Stop();
    bool IsEnabled() const { return enabled_; }
};

}
