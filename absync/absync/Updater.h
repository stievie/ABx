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
#include "HttpSource.h"
#include "Hash.h"

namespace Sync {

class FileDestination;

class Updater
{
public:
    struct RemoteFile
    {
        std::string name;
        std::string basePath;
        Sha1Hash hash;
    };
private:
    std::string localDir_;
    std::string indexFile_;
    std::unique_ptr<FileDestination> localBackend_;
    std::unique_ptr<HttpSource> remoteBackend_;
    std::unique_ptr<httplib::Headers> remoteHeaders_;
    std::vector<RemoteFile> remoteFiles_;
    size_t currentFile_{ 0 };
    bool cancelled_{ false };
    bool ProcessFile(const RemoteFile& file);
public:
    enum class ErrorType
    {
        Remote,
        Local
    };
    Updater(const std::string& remoteHost, uint16_t remotePort, const std::string remoteAuth,
        const std::string& localDir, const std::string& indexFile);
    bool Execute();
    void Cancel() { cancelled_ = true; }
    bool DownloadRemoteFiles();
    const std::vector<RemoteFile>& GetRemoteFiles() const { return remoteFiles_; }
    std::function<void(size_t fileIndex, size_t maxFiles, size_t value, size_t max)> onProgress_;
    std::function<bool(size_t fileIndex, size_t maxFiles, const std::string& filename)> onProcessFile_;
    std::function<void(const std::string& filename)> onFailure_;
    std::function<void(const std::string& filename, bool different, size_t downloaded, size_t copied, int savings)> onDoneFile_;
    std::function<void(ErrorType type, const char* message)> onError;
    std::function<void()> onUpdateStart_;
    std::function<void(bool success)> onUpdateDone_;
    std::function<void(size_t bps)> onDownloadProgress_;
};

}
