/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */



#if defined(AB_UNIX)

#include <sys/times.h>
#include "CpuUsage.h"
#include "Utils.h"

namespace System {

CpuUsage::CpuUsage() = default;

unsigned CpuUsage::GetUsage()
{
    unsigned nCpuCopy = cpuUsage_;
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

        cpuUsage_ = static_cast<unsigned>(percent);
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
