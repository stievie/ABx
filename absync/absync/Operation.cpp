/**
 * Copyright (c) 2018 Rokas Kupstys
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

#include "Operation.h"
#include <unordered_map>

namespace Sync {

static std::unordered_map<size_t, std::vector<const Boundary*>> CreateBoundaryLookupTable(const BoundaryList& boundary_list)
{
    std::unordered_map<size_t, std::vector<const Boundary*>> result;
    for (const auto& boundary : boundary_list)
        result[boundary.hash].emplace_back(&boundary);
    return result;
}

static bool Intersects(const Boundary& a, const Boundary& b)
{
    auto a_end = a.start + a.length;
    auto b_end = b.start + b.length;
    return (b.start <= a.start && a.start < b_end) || (a.start <= b.start && b.start < a_end);
}

OperationList CompareFiles(const BoundaryList& local, const BoundaryList& remote)
{
    OperationList result;
    auto local_file_table = CreateBoundaryLookupTable(local);

    // Iterate remote file and produce instructions to reassemble remote file from pieces available locally.
    for (const auto& block : remote)
    {
        enum class Status
        {
            NotFound,               // Block is not present in local file
            Copied,                 // Block is present in local file and can be copied
            Present,                // Block is present in local file at required location, no action needed
        } status = Status::NotFound;

        auto it_local_blocks = local_file_table.find(block.hash);
        if (it_local_blocks != local_file_table.end())
        {
            const Boundary* found = nullptr;
            for (const auto& local_block : it_local_blocks->second)
            {
                if (local_block->fingerprint == block.fingerprint && local_block->hash == block.hash && local_block->length == block.length)
                {
                    // Block was found in local file
                    if (local_block->start != block.start)
                    {
                        // Block was moved
                        found = local_block;
                        status = Status::Copied;
                    }
                    else
                    {
                        // Exactly same block at required position exists in a local file. No need to do anything.
                        status = Status::Present;
                        break;
                    }
                }
            }

            if (status == Status::Copied)
                result.push_back({ &block, found });
        }

        if (status == Status::NotFound)
        {
            // Block does not exist in local file. Download.
            result.push_back({ &block, nullptr });
        }
    }

    // Sort operations list
    for (auto i = 0UL; i < result.size(); i++)
    {
        for (auto j = i + 1; j < result.size(); j++)
        {
            auto& a = result[i];
            auto& b = result[j];

            // Solve circular dependencies, two blocks depending on each other's local data.
            if (a.local != nullptr && b.local != nullptr)
            {
                if (Intersects(*a.remote, *b.local) && Intersects(*b.remote, *a.local))
                {
                    // Force next block download
                    b.local = nullptr;
                }
            }

            // Make blocks with local source come before blocks that write to local source position
            if (b.local != nullptr && Intersects(*a.remote, *b.local))
            {
                result.insert(result.begin() + i, b);
                result.erase(result.begin() + j + 1);
            }
        }
    }

    return result;
}

}
