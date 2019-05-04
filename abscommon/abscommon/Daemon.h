#pragma once

#include <AB/CommonConfig.h>

#ifdef AB_UNIX

namespace System {

class DaemonBase
{
protected:
    virtual bool OnStart(int /* argc */, char* /* argv[] */) { return false; }
    virtual void OnStop() { }
public:
    bool running_ = false;
    bool Start(int argc, char* argv[]);
    void Stop();
};

template<T>
class Daemon : public DaemonBase
{
private:
    std::shared_ptr<T> app_;
    std::thread thread_;
public:
    bool OnStart(int argc, char* argv[]) final
    {
        app_ = std::make_shared<T>();
        if (!app_->InitializeA(dwArgc, pszArgv))
        {
            std::cout << "Application initialization failed" << sttd::endl;
            app_.reset();
            return false;
        }
        thread_ = std::thread(&T::Run, app_);
        return true;
    }
    void OnStop() final
    {
        if (app_)
        {
            app_->Stop();
            app_.reset();
            thread_.join();
        }
    }
};

}

// Creates the entry point
#define AB_DAEMON_MAIN(serviceName)                                            \
int main(int argc, char* argv[])                                               \
{                                                                              \
    Daemon<servieName> daemon;                                                 \    
    if (!daemon.Start(argc, argv))                                             \
        return EXIT_FAILURE;                                                   \
    while (daemon.running_)                                                    \
        sleep(10);                                                             \
    return EXIT_SUCCESS;                                                       \
}                              

#endif
