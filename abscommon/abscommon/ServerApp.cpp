#include "stdafx.h"
#include "ServerApp.h"
#if defined(__linux__) || defined(__unix__)
#include <unistd.h>
#endif

bool ServerApp::Initialize(int argc, char** argv)
{
#ifdef _WIN32
    char buff[MAX_PATH];
    GetModuleFileNameA(NULL, buff, MAX_PATH);
    exeFile_ = std::string(buff);
#else
    char buff[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", buff, PATH_MAX);
    exeFile_ = std::string(result, (count > 0) ? count : 0);
#endif
    size_t pos = exeFile_.find_last_of("\\/");
    path_ = exeFile_.substr(0, pos);
    for (int i = 1; i < argc; i++)
    {
        arguments_.push_back(std::string(argv[i]));
    }

    return true;
}
