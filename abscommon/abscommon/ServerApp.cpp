#include "stdafx.h"
#include "ServerApp.h"
#if !defined(_WIN32)
#include <unistd.h>
#endif

ServerApp* ServerApp::Instance = nullptr;

ServerApp::ServerApp()
{
    assert(ServerApp::Instance == nullptr);
    ServerApp::Instance = this;
}

ServerApp::~ServerApp()
{
}

bool ServerApp::Initialize(int argc, char** argv)
{
#ifdef _WIN32
    char buff[MAX_PATH];
    GetModuleFileNameA(NULL, buff, MAX_PATH);
    std::string aux(buff);
#else
    char buff[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", buff, PATH_MAX);
    std::string aux(result, (count > 0) ? count : 0);
#endif
    size_t pos = aux.find_last_of("\\/");
    path_ = aux.substr(0, pos);
    for (int i = 0; i < argc; i++)
    {
        arguments_.push_back(std::string(argv[i]));
    }

    return true;
}
