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
#include "SyncDefs.h"

namespace Sync {

Synchronizer::Synchronizer(LocalBackend& local, RemoteBackend& remote) :
    local_(local),
    remote_(remote)
{}

bool Synchronizer::Synchronize(const std::string& localFile, const std::string& remoteFile)
{
    // Calculate hashes for local file
    const auto localHashes = PartitionFile(localFile, {});

    // Download JSON from remote (<filename>.META_FILE_EXT)
    const auto remoteJson = remote_.GetChunk(remoteFile + META_FILE_EXT);
    if (remoteJson.size() == 0)
        // Does not exist on server
        return false;
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
            remote_.GetChunk(remoteFile, op.remote->start, op.remote->length) :
            // Copy
            local_.GetChunk(localFile, op.local->start, op.local->length);
        if (buffer.size() != op.remote->length)
            return false;
        if (op.local == nullptr)
            downloaded_ += op.remote->length;
        else
            copied_ += op.remote->length;
        if (!local_.WriteChunk(localFile, buffer, op.remote->start, op.remote->length))
            return false;
        ++i;
        if (!CallProgress(i, delta.size()))
            return false;
    }

    filesize_ = remoteHashes.back().start + remoteHashes.back().length;
    local_.Truncate(localFile, filesize_);

    return true;
}

bool Synchronizer::CallProgress(size_t value, size_t max)
{
    if (!onProgress_)
        return true;
    return onProgress_(value, max);
}

}
