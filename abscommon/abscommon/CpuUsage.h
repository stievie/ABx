#pragma once

#if defined(AB_WINDOWS)

#include <windows.h>

namespace System {

class CpuUsage
{
public:
    CpuUsage();

    short GetUsage();
private:
    ULONGLONG SubtractTimes(const FILETIME& ftA, const FILETIME& ftB);
    bool EnoughTimePassed();
    inline bool IsFirstRun() const
    {
        return (lastRun_ == 0);
    }

    //system total times
    FILETIME prevSysKernel_;
    FILETIME prevSysUser_;

    //process times
    FILETIME prevProcKernel_;
    FILETIME prevProcUser_;

    short cpuUsage_;
    ULONGLONG lastRun_;

    volatile LONG runCount_;
};

}

#endif