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
#include <stdint.h>
#include <memory>
#include "HttpRemoteBackend.h"

namespace Sync {

class FileLocalBackend;

class Updater
{
private:
    struct RemoteFile
    {
        std::string name;
        std::string hash;
    };
    std::string localDir_;
    std::unique_ptr<FileLocalBackend> localBackend_;
    std::unique_ptr<HttpRemoteBackend> remoteBackend_;
    std::unique_ptr<httplib::Headers> remoteHeaders_;
    std::vector<RemoteFile> remoteFiles_;
    size_t currentFile_{ 0 };
    bool GetRemoteFiles();
    bool ProcessFile(const RemoteFile& file);
public:
    Updater(const std::string& remoteHost, uint16_t remotePort, const std::string remoteAuth,
        const std::string& localDir);
    bool Execute();
    std::function<void(size_t fileValue, size_t fileMax, size_t value, size_t max)> onProgress_;
    std::function<bool(const std::string& filename)> onProcessFile_;
    std::function<void(const std::string& filename)> onFailure_;
    std::function<void(const std::string& filename, bool different, size_t downloaded, size_t copied, int savings)> onDoneFile_;
};

}
