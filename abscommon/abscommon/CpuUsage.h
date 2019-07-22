#pragma once

#if defined(AB_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <windows.h>
#endif

namespace System {

class CpuUsage
{
public:
    CpuUsage();

    /// Returns CPU usage of the calling process in percent, i.e. a value between 0..100.
    short GetUsage();
private:
    short cpuUsage_{ -1 };
#if defined(AB_WINDOWS)
    ULONGLONG SubtractTimes(const FILETIME& ftA, const FILETIME& ftB);

    //system total times
    FILETIME prevSysKernel_;
    FILETIME prevSysUser_;

    //process times
    FILETIME prevProcKernel_;
    FILETIME prevProcUser_;

    ULONGLONG lastRun_{ 0 };

    volatile LONG runCount_;
#elif defined(AB_UNIX)
    int64_t lastRun_{ 0 };
    clock_t lastCPU_{ 0 };
    clock_t lastSysCPU_{ 0 };
    clock_t lastUserCPU_{ 0 };
#endif
    bool EnoughTimePassed();
    inline bool IsFirstRun() const
    {
        return (lastRun_ == 0);
    }
};

}
