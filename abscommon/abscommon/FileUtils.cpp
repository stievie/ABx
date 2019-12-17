#include "stdafx.h"
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
    std::ifstream fsrc(src, std::ios::binary);
    if (!fsrc.is_open())
        return false;
    std::ofstream fdst(dst, std::ios::binary);
    if (!fdst.is_open())
        return false;
    fdst << fsrc.rdbuf();
    return true;
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
#if defined(_WIN32)
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
