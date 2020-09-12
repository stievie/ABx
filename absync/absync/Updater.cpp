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

#include "Updater.h"
#include "LocalBackend.h"
#include <map>
#include <pugixml.hpp>
#include "Synchronizer.h"
#include <filesystem>
#include "Hash.h"

namespace fs = std::filesystem;

namespace Sync {

Updater::Updater(const std::string& remoteHost, uint16_t remotePort, const std::string remoteAuth,
    const std::string& localDir) :
    localDir_(localDir)
{
    remoteHeaders_ = std::make_unique<httplib::Headers>();
    if (!remoteAuth.empty())
        remoteHeaders_->emplace("Auth", remoteAuth);
    remoteBackend_ = std::make_unique<Sync::HttpRemoteBackend>(remoteHost, remotePort, *remoteHeaders_);
    remoteBackend_->onError_ = [this](const char* message)
    {
        if (onError)
            onError(ErrorType::Remote, message);
    };
    localBackend_ = std::make_unique<Sync::FileLocalBackend>();
    localBackend_->onError_ = [this](const char* message)
    {
        if (onError)
            onError(ErrorType::Local, message);
    };
}

bool Updater::Execute()
{
    if (remoteFiles_.size() == 0)
    {
        if (onUpdateStart_)
            onUpdateStart_();
        if (!DownloadRemoteFiles())
            return false;
    }
    else
    {
        if (onUpdateStart_)
            onUpdateStart_();
    }

    bool result = true;
    for (const auto& file : remoteFiles_)
    {
        if (cancelled_)
            return false;

        ++currentFile_;
        if (onProcessFile_)
        {
            if (!onProcessFile_(file.name))
                continue;
        }
        if (!ProcessFile(file))
            result = false;
    }
    if (onUpdateDone_)
        onUpdateDone_(result);
    return result;
}

bool Updater::ProcessFile(const RemoteFile& file)
{
    const auto rootPath = fs::canonical(localDir_);
    const auto localFile = rootPath / fs::path(file.name);
    const std::string localHash = HashFile(localFile.string());
    if (strcasecmp(localHash.c_str(), file.hash.c_str()) == 0)
    {
        if (onDoneFile_)
            onDoneFile_(file.name, false, 0, 0, 0);
        return true;
    }
    Sync::Synchronizer sync(*localBackend_, *remoteBackend_);
    sync.onProgress_ = [this](size_t value, size_t max) -> bool
    {
        if (onProgress_)
            onProgress_(currentFile_, remoteFiles_.size(), value, max);
        return !cancelled_;
    };
    if (!sync.Synchronize(localFile.string(), file.name))
    {
        if (onFailure_)
            onFailure_(file.name);
        return false;
    }
    if (cancelled_)
        return false;
    if (onDoneFile_)
        onDoneFile_(file.name, sync.IsDifferent(), sync.GetDownloaded(), sync.GetCopied(), sync.GetSavings());
    return true;
}

bool Updater::DownloadRemoteFiles()
{
    const auto files = remoteBackend_->GetChunk("/_files_");
    pugi::xml_document doc;
    const pugi::xml_parse_result result = doc.load_buffer(files.data(), files.size());
    if (result.status != pugi::status_ok)
        return false;
    const pugi::xml_node filesNode = doc.child("files");
    if (!filesNode)
        return false;
    for (pugi::xml_node_iterator it = filesNode.begin(); it != filesNode.end(); ++it)
    {
        if (strcmp((*it).name(), "file") == 0)
        {
            const pugi::xml_attribute& pathAttr = it->attribute("path");
            const pugi::xml_attribute& sha1Attr = it->attribute("sha1");
            remoteFiles_.push_back({ pathAttr.as_string(), sha1Attr.as_string() });
        }
    }
    return true;
}


}
