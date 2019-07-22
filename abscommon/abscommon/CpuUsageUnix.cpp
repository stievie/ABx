#include "stdafx.h"

#if defined(AB_UNIX)

#include <sys/times.h>
#include "CpuUsage.h"
#include "Utils.h"

namespace System {

CpuUsage::CpuUsage() = default;

short CpuUsage::GetUsage()
{
    short nCpuCopy = cpuUsage_;
    if (!EnoughTimePassed())
        return nCpuCopy;

    if (!IsFirstRun())
    {
        double percent;
        struct tms buf;
        clock_t now = times(&buf);
        if (now <= lastCPU_ || buf.tms_stime < lastSysCPU_ || buf.tms_utime < lastUserCPU_)
            percent = -1.0;
        else
        {
            percent = (buf.tms_stime - lastSysCPU_) + (buf.tms_utime - lastUserCPU_);
            percent /= (now - lastCPU_);
//            percent /= numProcessors;
            percent *= 100;
        }
        lastCPU_ = now;
        lastSysCPU_ = buf.tms_stime;
        lastUserCPU_ = buf.tms_utime;

        cpuUsage_ = static_cast<short>(percent);
        nCpuCopy = cpuUsage_;
    }

    lastRun_ = Utils::Tick();

    return nCpuCopy;
}

bool CpuUsage::EnoughTimePassed()
{
    static const unsigned minElapsedMS = 250;   // milliseconds
    return Utils::TimeElapsed(lastRun_) > minElapsedMS;
}

}

#endif
