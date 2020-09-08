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

#include "Synchronizer.h"
#include "Partition.h"
#include "Operation.h"
#include <fstream>

namespace Sync {

static std::string ExtractFileName(const std::string& fn)
{
    size_t pos = fn.find_last_of("\\/");
    if (pos != std::string::npos)
        return fn.substr(pos + 1);
    return fn;
}

Synchronizer::Synchronizer(LocalBackend& local, RemoteBackend& remote) :
    local_(local),
    remote_(remote)
{}

bool Synchronizer::Synchronize(const std::string& file)
{
    // Calculate hashes for local file
    const auto localHashes = PartitionFile(file, {});

    // Download JSON from remote (<filename>.json)
    const auto remoteJson = remote_.GetChunk("/" + ExtractFileName(file) + ".json");
    const auto remoteHashes = LoadBoundaryList(remoteJson);

    const auto delta = CompareFiles(localHashes, remoteHashes);
    different_ = delta.size() > 0;
    if (delta.size() == 0)
        return true;

    size_t i = 0;
    for (const auto& op : delta)
    {
        const std::vector<char> buffer = (op.local == nullptr) ?
            // Download
            remote_.GetChunk("/" + ExtractFileName(file), op.remote->start, op.remote->length) :
            // Copy
            local_.GetChunk(file, op.local->start, op.local->length);
        if (buffer.size() == 0)
            return false;
        if (op.local == nullptr)
            downloaded_ += op.remote->length;
        else
            copied_ += op.remote->length;
        local_.WriteChunk(file, buffer, op.remote->start, op.remote->length);
        ++i;
        CallProgress(i, delta.size());
    }

    filesize_ = remoteHashes.back().start + remoteHashes.back().length;
    local_.Truncate(file, filesize_);

    return true;
}

void Synchronizer::CallProgress(size_t value, size_t max)
{
    if (!onProgress_)
        return;
    onProgress_(value, max);
}

}