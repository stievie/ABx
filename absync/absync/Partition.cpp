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

#include "Partition.h"
#include <fstream>
#include <sa/Assert.h>
extern "C" {
#include <abcrypto/buzhash.h>
}
#include <algorithm>
#include <sa/StringHash.h>
#include <json.hpp>

namespace Sync {

static void PartitionChunk(std::ifstream& ifs, size_t startOffset, size_t endOffset, BoundaryList& list, const Parameters& params)
{
    ASSERT(startOffset < endOffset);
    size_t length = endOffset - startOffset;
    std::vector<char> buffer(params.bufferSize);

    const auto mask = (1U << params.matchBits) - 1U;

    size_t bufferSize = std::min(length, params.bufferSize);
    ifs.seekg(startOffset, std::ios::beg);
    std::streamsize readLength = ifs.read(&buffer[0], static_cast<std::streamsize>(bufferSize)).gcount();
    if ((size_t)readLength != bufferSize)
        return;

    auto* data = &buffer[0];
    auto* dataStart = data;
    size_t windowLength = params.windowLength;
    uint32_t fingerprint = buzhash((const unsigned char*)data, windowLength);

    BoundaryList localResult;

    do
    {
        for (auto* end = dataStart + bufferSize - windowLength; data < end;)
        {
            if ((fingerprint & mask) == 0)
            {
                auto blockStart = startOffset + (data - dataStart);
                localResult.push_back({ blockStart, fingerprint, 0, 0 });
            }

            fingerprint = buzhash_update(fingerprint, data[0], data[windowLength], windowLength);
            data++;
        }

        auto bytesConsumed = bufferSize - std::min<size_t>(windowLength, bufferSize);
        length -= bytesConsumed;
        startOffset += bytesConsumed;

        bufferSize = std::min<int64_t>(length, params.bufferSize);
        ifs.seekg(startOffset, std::ios::beg);
        readLength = ifs.read(&buffer[0], static_cast<std::streamsize>(bufferSize)).gcount();
        if ((size_t)readLength != bufferSize)
            return;
        data = dataStart;
    } while (length > windowLength);

    list.insert(list.end(), localResult.begin(), localResult.end());
}

static void SplitBlocks(std::ifstream& ifs, BoundaryList& list, const Parameters& params)
{
    size_t prev_offset = 0L;
    std::vector<char> buffer(params.windowLength);

    for (auto it = list.begin(); it != list.end(); ++it)
    {
        auto& split = *it;
        size_t block_size = split.start - prev_offset;
        prev_offset = split.start;
        if (block_size > params.maxBlockSize)
        {
            size_t new_blocks_count = block_size / params.maxBlockSize;
            size_t new_block_size = block_size / (new_blocks_count + 1);
            BoundaryList splits;

            for (size_t i = 1UL; i < new_blocks_count + 1; ++i)
            {
                auto start = split.start - (i * new_block_size);
                ifs.seekg(start, std::ios::beg);
                auto len = ifs.read(&buffer[0], static_cast<std::streamsize>(params.windowLength)).gcount();
                auto fingerprint = buzhash((const unsigned char*)&buffer[0], static_cast<uint32_t>(len));
                splits.push_back({ start, fingerprint, 0, 0 });
            }

            it = list.insert(it, splits.rbegin(), splits.rend()) + splits.size();
        }
    }
}

static void CalculateBlockHashes(std::ifstream& ifs, BoundaryList& list)
{
    std::vector<char> wbuffer;
    for (auto& block : list)
    {
        ifs.seekg(block.start, std::ios::beg);
        if (wbuffer.size() < static_cast<size_t>(block.length))
            wbuffer.resize(static_cast<size_t>(block.length));
        auto len = ifs.read(&wbuffer[0], static_cast<std::streamsize>(block.length)).gcount();
        block.hash = sa::StringHashRt(&wbuffer[0], (size_t)len);
    }
}

BoundaryList PartitionFile(const std::string& filename, const Parameters& params)
{
    std::ifstream ifs(filename, std::ifstream::in | std::ios::binary | std::ios::ate);
    ifs.seekg(0, std::ios::end);
    size_t fileSize = static_cast<size_t>((long)ifs.tellg());
    if (fileSize == 0)
        return {};

    BoundaryList result;
    result.push_back({ });
    PartitionChunk(ifs, 0, fileSize, result, params);
    std::sort(result.begin(), result.end(), [](const Boundary& a, const Boundary& b) { return a.start < b.start; });

    if (result.size() > 1)
    {
        // Remove blocks that are too small
        auto next_offset = fileSize;
        for (ssize_t i = result.size() - 1; i >= 0; --i)
        {
            auto& split = result[i];
            if (next_offset - split.start < params.minBlockSize)
                result.erase(result.begin() + i);
            next_offset = split.start;
        }
    }

    SplitBlocks(ifs, result, params);

    {
        auto first_block_size = result.size() > 1 ? result[1].start : fileSize;
        std::vector<char> buffer(first_block_size);

        if (result.size() == 0)
            result.push_back({ });

        ifs.seekg(0, std::ios::beg);
        auto len = ifs.read(&buffer[0], static_cast<std::streamsize>(params.windowLength)).gcount();
        result[0].fingerprint = buzhash((const unsigned char*)&buffer[0], static_cast<size_t>(len));

        ifs.seekg(0, std::ios::beg);
        len = ifs.read(&buffer[0], static_cast<std::streamsize>(first_block_size)).gcount();
        result[0].hash = sa::StringHashRt(&buffer[0], static_cast<size_t>(len));
    }

    for (size_t i = 0, end = result.size(); i < end; i++)
    {
        auto& block = result[i];
        auto next_block_start = i + 1 < result.size() ? result[i + 1].start : fileSize;
        block.length = next_block_start - block.start;
    }

    CalculateBlockHashes(ifs, result);

    return result;
}

void SaveBoundaryList(const std::string& filename, const BoundaryList& list)
{
    json::JSON obj;

    obj["boundaries"] = json::Array();
    for (const auto& b : list)
    {
        auto bd = json::Object();
        bd["start"] = b.start;
        bd["length"] = b.length;
        bd["hash"] = b.hash;
        bd["fingerprint"] = b.fingerprint;
        obj["boundaries"].append(bd);
    }
    std::ofstream out(filename + META_FILE_EXT);
    out << obj << std::endl;
}

BoundaryList LoadBoundaryList(const std::vector<char>& buffer)
{
    if (buffer.size() == 0)
        return {};
    std::string_view sv(&buffer[0], buffer.size());
    return LoadBoundaryList(sv);
}

BoundaryList LoadBoundaryList(const std::string_view buffer)
{
    const json::JSON obj = json::JSON::Load(buffer);
    if (!obj.hasKey("boundaries"))
        return {};

    BoundaryList result;
    const auto boundaries = obj.at("boundaries").ArrayRange();
    for (const auto& b : boundaries)
    {
        result.push_back({
            (size_t)b.at("start").ToInt(),
            (uint32_t)b.at("fingerprint").ToInt(),
            (size_t)b.at("hash").ToInt(),
            (size_t)b.at("length").ToInt()
        });
    }
    return result;
}

BoundaryList LoadBoundaryList(const std::string& filename)
{
    std::ifstream ifs(filename + ".json", std::ifstream::in);
    if (!ifs)
        return {};
    ifs.seekg(0, std::ios::end);
    size_t fileSize = static_cast<size_t>((long)ifs.tellg());
    std::string json;
    json.resize(fileSize);
    ifs.seekg(0, std::ios::beg);
    auto len = ifs.read(&json[0], static_cast<std::streamsize>(fileSize)).gcount();
    if (static_cast<size_t>(len) != fileSize)
        return {};
    return LoadBoundaryList(json);
}

}
