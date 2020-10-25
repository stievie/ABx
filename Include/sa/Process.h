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

#include <sa/Compiler.h>
#include <sa/StringTempl.h>
#include <string>
#if defined(SA_PLATFORM_WIN)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif defined(SA_PLATFORM_LINUX) || defined(SA_PLATFORM_UNIX)
#include <spawn.h>
#include <unistd.h>
#if defined(SA_PLATFORM_LINUX)
#include <linux/limits.h>
#endif
#else
#error "Unsupported platform"
#endif
#include <sstream>
#include <vector>

namespace sa {

// Process helper for Windows and Linux
class Process
{
private:
    std::vector<std::string> arguments_;
public:
    explicit Process(const std::vector<std::string>& arguments) :
        arguments_(arguments)
    {
    }
    Process(int argc, char** argv)
    {
        for (int i = 1; i < argc; ++i)
            arguments_.push_back(argv[i]);
    }
    static bool Run(const std::string& command)
    {
#if defined(SA_PLATFORM_WIN)
        STARTUPINFOA info = { sizeof(info) };
        PROCESS_INFORMATION processInfo = { 0 };
        char* cmd = (char*)command.c_str();
        if (CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &info, &processInfo))
            return true;
        return false;
#elif defined(SA_PLATFORM_LINUX) || defined(SA_PLATFORM_UNIX)
        char* cmd = (char*)command.c_str();
        pid_t pid;
        char* argv[] = { (char*)"sh", (char*)"-c", cmd, NULL };
        int status = posix_spawn(&pid, "/bin/sh", NULL, NULL, argv, environ);
        if (status != 0)
            return false;
        return true;
#endif
    }
    static std::string GetSelf()
    {
#ifdef SA_PLATFORM_WIN
        char buff[MAX_PATH];
        GetModuleFileNameA(NULL, buff, MAX_PATH);
        return std::string(buff);
#elif defined(SA_PLATFORM_LINUX) || defined(SA_PLATFORM_UNIX)
        char buff[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", buff, PATH_MAX);
        return std::string(buff, (count > 0) ? count : 0);
#endif
    }
    static bool IsSelf(const std::string file)
    {
        std::string testFile(file);
        ReplaceSubstring<char>(testFile, "\\", "/");
        std::string self(GetSelf());
        ReplaceSubstring<char>(self, "\\", "/");
#ifdef SA_PLATFORM_WIN
        return testFile.size() == self.size()
            && std::equal(testFile.cbegin(), testFile.cend(), self.cbegin(),
                [](std::string::value_type l1, std::string::value_type r1)
        {
            return toupper(l1) == toupper(r1);
        });
#else
        return testFile.compare(self) == 0;
#endif
    }
    static std::string GetSelfPath()
    {
        const std::string self = GetSelf();
        auto pos = self.find_last_of("/\\");
        if (pos == std::string::npos)
            return "";
        return self.substr(0, pos);
    }
    static std::string GetCurrentDir()
    {
#ifdef SA_PLATFORM_WIN
        char buff[MAX_PATH];
        auto len = GetCurrentDirectoryA(MAX_PATH, buff);
        if (len == 0)
            return "";
        return std::string(buff, (size_t)len);
#elif defined(SA_PLATFORM_LINUX) || defined(SA_PLATFORM_UNIX)
        char buff[PATH_MAX];
        if (getcwd(buff, PATH_MAX) == nullptr)
        {
            perror("getcwd");
            return "";
        }
        return std::string(buff);
#endif
    }
    static bool SetCurrentDir(const std::string& dir)
    {
#ifdef SA_PLATFORM_WIN
        return SetCurrentDirectoryA(dir.c_str());
#elif defined(SA_PLATFORM_LINUX) || defined(SA_PLATFORM_UNIX)
        int ret = chdir(dir.c_str());
        if (ret < 0)
            perror("chdir");
        return ret == 0;
#endif
    }
    std::string GetArguments() const
    {
        std::stringstream ss;
        for (size_t i = 0; i < arguments_.size(); ++i)
        {
            const std::string& sv(arguments_[i]);
            const size_t spacepos = sv.find(' ');
            const size_t tabpos = sv.find('\t');
            if (spacepos != std::string::npos || tabpos != std::string::npos)
                ss << "\"";
            ss << sv;
            if (spacepos != std::string::npos || tabpos != std::string::npos)
                ss << "\"";
            if (i < arguments_.size() -1)
                ss << " ";
        }
        return ss.str();
    }
    void Restart()
    {
        const std::string cmd = "\"" + GetSelf() + "\"" + " " + GetArguments();
        if (Run(cmd))
        {
            exit(0);
        }
    }
};

}
