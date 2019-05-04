#include "stdafx.h"
#include "Daemon.h"

#ifdef AB_UNIX

namespace System {

bool DaemonBase::Start(int argc, char* argv[])
{
    if (!OnStart(argc, argv))
        return false;
    running_ = true;
    return true;
}

void DaemonBase::Stop()
{
    OnStop();
    running_ = false;
}

}

#endif
