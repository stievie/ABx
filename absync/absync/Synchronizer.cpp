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

Synchronizer::Synchronizer(LocalBackend& local, RemoteBackend& remote) :
    local_(local),
    remote_(remote)
{}

bool Synchronizer::Synchronize(const std::string& file)
{
    // Calculate hashes for local file
    const auto localHashes = PartitionFile(file, {});

    // Download JSON from remote (<filename>.json)
    const auto remoteJson = remote_.GetChunk(file + ".json");
    const auto remoteHashes = LoadBoundaryList(remoteJson);

    const auto delta = CompareFiles(localHashes, remoteHashes);
    if (delta.size() == 0)
        return true;

    for (const auto& op : delta)
    {
        const std::vector<char> buffer = (op.local == nullptr) ?
            // Download
            remote_.GetChunk(file, op.remote->start, op.remote->length) :
            // Copy
            local_.GetChunk(file, op.remote->start, op.remote->length);
        local_.WriteChunk(file, buffer, op.remote->start, op.remote->length);
    }

    auto file_size = remoteHashes.back().start + remoteHashes.back().length;
    local_.Truncate(file, file_size);

    return true;
}

}