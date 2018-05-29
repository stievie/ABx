#pragma once

class ServerApp
{
public:
    ServerApp() = default;
    virtual ~ServerApp() = default;
    virtual bool Initialize(int argc, char** argv);
    virtual void Run()
    { }
    virtual void Stop()
    { }

    std::string path_;
    std::string exeFile_;
    std::vector<std::string> arguments_;
};
