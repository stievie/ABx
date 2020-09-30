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
#include <cctype>
#include <sa/Compiler.h>

namespace sa {

class path
{
private:
    std::string path_;
public:
    path() { }
    path(const std::string& path) :
        path_(path)
    { }
    path(const char* path) :
        path_(path)
    { }
    // No traling slash
    std::string directory() const
    {
        size_t pos = path_.find_last_of("\\/");
        return path_.substr(0, pos);
    }
    void set_directory(const std::string& dir)
    {
        path_ = (path(dir) / path(filename())).string();
    }
    std::string filename() const
    {
        size_t pos = path_.find_last_of("\\/");
        if (pos != std::string::npos)
            return path_.substr(pos + 1);
        return path_;
    }
    void set_filename(const std::string& fn)
    {
        path_ = (path(directory()) / path(fn)).string();
    }
    std::string ext() const
    {
        size_t pos = path_.find_last_of('.');
        if (pos != std::string::npos)
            return path_.substr(pos);
        return "";
    }
    void set_ext(const std::string& ext)
    {
        size_t pos = path_.find_last_of('.');
        if (pos != std::string::npos)
            path_ = path_.substr(0, pos) + ext;
        else
            path_ += ext;
    }
    path operator + (const path& rhs) const
    {
        return path(path_ + rhs.path_);
    }
    path operator + (const std::string& rhs) const
    {
        return path(path_ + rhs);
    }
    path operator / (const path& rhs) const
    {
        path result(path_);
        if (result.path_.back() != '/' && result.path_.back() != '\\' && rhs.path_.front() != '/' && rhs.path_.front() != '\\')
            result.path_ += "/";
        result.path_ += rhs.path_;
        return result;
    }
    bool operator != (const path& rhs) const
    {
        return !(*this == rhs);
    }
    bool operator == (const path& rhs) const
    {
#ifdef SA_PLATFORM_WIN
        return path_.size() == rhs.path_.size()
            && std::equal(path_.cbegin(), path_.cend(), rhs.path_.cbegin(),
                [](std::string::value_type l1, std::string::value_type r1)
        {
            return toupper(l1) == toupper(r1);
        });
#else
        return path_.compare(rhs.path_) == 0;
#endif
    }
    const std::string& string() const { return path_; }
};

}