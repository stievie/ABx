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

#include <vector>
#include <string>
#include <fstream>
#include <functional>

namespace Sync {

class LocalBackend
{
protected:
    virtual void Error(const char* message)
    {
        if (onError_)
            onError_(message);
    }
public:
    virtual std::vector<char> GetChunk(const std::string& filename, size_t start = 0, size_t length = 0) = 0;
    virtual bool WriteChunk(const std::string& filename, const std::vector<char>& data, size_t start, size_t length) = 0;
    virtual void Truncate(const std::string& filename, size_t size) = 0;
    std::function<void(const char*)> onError_;
};

// Default local backend taking absolute filenames
class FileLocalBackend : public LocalBackend
{
private:
    std::string filename_;
    std::fstream stream_;
    bool OpenFile(const std::string& filename);
public:
    std::vector<char> GetChunk(const std::string& filename, size_t start = 0, size_t length = 0) override;
    bool WriteChunk(const std::string& filename, const std::vector<char>& data, size_t start, size_t length) override;
    void Truncate(const std::string& filename, size_t size) override;
};

}
