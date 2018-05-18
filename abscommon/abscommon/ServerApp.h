#pragma once

class ServerApp
{
public:
    ServerApp();
    virtual ~ServerApp();
    virtual bool Initialize(int argc, char** argv);
    virtual void Run()
    { }
    virtual void Stop()
    { }

    std::string path_;
    std::vector<std::string> arguments_;

    static ServerApp* Instance;
};

