#include "stdafx.h"
#include "Service.h"
#include <functional>

void ServiceManager::Run()
{
    assert(!running_);

}

void ServiceManager::Stop()
{
    if (!running_)
        return;

    for (const auto& sp : acceptors_)
    {
        ioService_.post(std::bind(&ServicePort::OnStopServer, sp.second));
    }
    acceptors_.clear();
}

void ServiceManager::Die()
{
    ioService_.stop();
}

void ServicePort::OnStopServer()
{
}
