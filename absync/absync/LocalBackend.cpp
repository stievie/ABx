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

#include "LocalBackend.h"
#include <sa/Assert.h>
#include <filesystem>

namespace Sync {

bool FileLocalBackend::OpenFile(const std::string& filename)
{
    if (filename == filename_ && stream_.is_open())
        return true;
    stream_ = std::fstream(filename, std::fstream::binary | std::fstream::in | std::fstream::out);
    if (!stream_.is_open())
        // New file
        stream_ = std::fstream(filename, std::fstream::binary | std::fstream::out);
    filename_ = filename;
    bool result = stream_.is_open();
    if (!result)
        Error("Error opening file");
    return result;
}

std::vector<char> FileLocalBackend::GetChunk(const std::string& filename, size_t start, size_t length)
{
    if (!OpenFile(filename))
        return {};
    std::vector<char> result(length);

    stream_.seekg(start, std::ios::beg);
    auto len = stream_.read(&result[0], static_cast<std::streamsize>(length)).gcount();
    ASSERT(static_cast<size_t>((long)len) == length);

    return result;
}

bool FileLocalBackend::WriteChunk(const std::string& filename, const std::vector<char>& data, size_t start, size_t length)
{
    if (!OpenFile(filename))
        return false;
    stream_.seekg(start, std::ios::beg);
    stream_.write(&data[0], length);
    return true;
}

void FileLocalBackend::Truncate(const std::string& filename, size_t size)
{
    std::filesystem::resize_file(filename, size);
}

}
