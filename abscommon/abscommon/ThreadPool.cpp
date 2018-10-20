#include "stdafx.h"
#include "ThreadPool.h"

namespace Asynch {

ThreadPool::~ThreadPool()
{
    Stop();
}

void ThreadPool::Start()
{
    if (pool_)
        return;

    pool_ = std::make_unique<thread_pool>(numThreads_);
}

void ThreadPool::Stop()
{
    if (pool_)
        pool_.reset();
}

}
