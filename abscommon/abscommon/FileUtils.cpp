/**
 * Copyright 2017-2020 Stefan Ascher
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


#include "FileUtils.h"
#include <fstream>
#include <sa/StringTempl.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <linux/limits.h>
#include <unistd.h>
#endif

namespace Utils {

bool FileCopy(const std::string& src, const std::string& dst)
{
#ifdef AB_WINDOWS
    return ::CopyFileA(src.c_str(), dst.c_str(), FALSE);
#else
    std::ifstream fsrc(src, std::ios::binary);
    if (!fsrc.is_open())
        return false;
    std::ofstream fdst(dst, std::ios::binary);
    if (!fdst.is_open())
        return false;
    fdst << fsrc.rdbuf();
    return true;
#endif
}

std::string NormalizeFilename(const std::string& filename)
{
    std::string normal_name(filename);
    sa::ReplaceSubstring<char>(normal_name, "\\", "/");
    while (sa::ReplaceSubstring<char>(normal_name, "//", "/"));
    if (normal_name.size() && normal_name[0] != '/')
        normal_name = "/" + normal_name;
    return normal_name;
}

bool FileExists(const std::string& name)
{
#ifdef AB_WINDOWS
    DWORD dwAttrib = ::GetFileAttributesA(name.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    std::ifstream infile(name.c_str());
    return infile.good();
#endif
}

std::string AddSlash(const std::string& dir)
{
    if (dir.back() != '\\' && dir.back() != '/')
        return dir + "/";
    return dir;
}

bool IsHiddenFile(const std::string& path)
{
    if (path.empty())
        return false;
    if ((path != "..") &&
        (path != ".") &&
        ((path[0] == '.') || (path.find("/.") != std::string::npos)))
        return true;
#if defined(AB_WINDOWS)
    DWORD attributes = ::GetFileAttributesA(path.c_str());
    if (attributes & FILE_ATTRIBUTE_HIDDEN)
        return true;
    if (attributes & FILE_ATTRIBUTE_SYSTEM)
        return true;
#endif
    return false;
}

std::string GetExeName()
{
#ifdef AB_WINDOWS
    char buff[MAX_PATH];
    GetModuleFileNameA(NULL, buff, MAX_PATH);
    return std::string(buff);
#else
    char buff[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", buff, PATH_MAX);
    return std::string(buff, (count > 0) ? count : 0);
#endif
}

}
