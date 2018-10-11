#include "stdafx.h"
#include "DefaultThreadPool.h"

namespace Asynch {

DefaultThreadPool DefaultThreadPool::Instance;

DefaultThreadPool::~DefaultThreadPool()
{
    Stop();
}

void DefaultThreadPool::Start()
{
    if (pool_)
        return;

    pool_ = std::make_unique<ThreadPool>(numThreads_);
}

void DefaultThreadPool::Stop()
{
    if (pool_)
        pool_.reset();
}

}
