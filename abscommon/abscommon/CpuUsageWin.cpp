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



#if defined(AB_WINDOWS)

#include "CpuUsage.h"
#include "Utils.h"

namespace System {

CpuUsage::CpuUsage() :
    runCount_(0)
{
    ZeroMemory(&prevSysKernel_, sizeof(FILETIME));
    ZeroMemory(&prevSysUser_, sizeof(FILETIME));

    ZeroMemory(&prevProcKernel_, sizeof(FILETIME));
    ZeroMemory(&prevProcUser_, sizeof(FILETIME));
}

/**********************************************
* CpuUsage::GetUsage
* returns the percent of the CPU that this process
* has used since the last time the method was called.
* If there is not enough information, -1 is returned.
* If the method is recalled to quickly, the previous value
* is returned.
***********************************************/
unsigned CpuUsage::GetUsage()
{
    //create a local copy to protect against race conditions in setting the
    //member variable
    unsigned nCpuCopy = cpuUsage_;
    if (::InterlockedIncrement(&runCount_) == 1)
    {
        /*
        If this is called too often, the measurement itself will greatly affect the
        results.
        */

        if (!EnoughTimePassed())
        {
            ::InterlockedDecrement(&runCount_);
            return nCpuCopy;
        }

        FILETIME ftSysIdle, ftSysKernel, ftSysUser;
        FILETIME ftProcCreation, ftProcExit, ftProcKernel, ftProcUser;

        if (!GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser) ||
            !GetProcessTimes(GetCurrentProcess(), &ftProcCreation, &ftProcExit, &ftProcKernel, &ftProcUser))
        {
            ::InterlockedDecrement(&runCount_);
            return nCpuCopy;
        }

        if (!IsFirstRun())
        {
            /*
            CPU usage is calculated by getting the total amount of time the system has operated
            since the last measurement (made up of kernel + user) and the total
            amount of time the process has run (kernel + user).
            */
            ULONGLONG ftSysKernelDiff = SubtractTimes(ftSysKernel, prevSysKernel_);
            ULONGLONG ftSysUserDiff = SubtractTimes(ftSysUser, prevSysUser_);

            ULONGLONG ftProcKernelDiff = SubtractTimes(ftProcKernel, prevProcKernel_);
            ULONGLONG ftProcUserDiff = SubtractTimes(ftProcUser, prevProcUser_);

            ULONGLONG nTotalSys = ftSysKernelDiff + ftSysUserDiff;
            ULONGLONG nTotalProc = ftProcKernelDiff + ftProcUserDiff;

            if (nTotalSys > 0)
                cpuUsage_ = static_cast<unsigned>((100.0 * nTotalProc) / nTotalSys);
        }

        prevSysKernel_ = ftSysKernel;
        prevSysUser_ = ftSysUser;
        prevProcKernel_ = ftProcKernel;
        prevProcUser_ = ftProcUser;

        lastRun_ = Utils::Tick();

        nCpuCopy = cpuUsage_;
    }

    ::InterlockedDecrement(&runCount_);

    return nCpuCopy;
}

ULONGLONG CpuUsage::SubtractTimes(const FILETIME& ftA, const FILETIME& ftB)
{
    LARGE_INTEGER a, b;
    a.LowPart = ftA.dwLowDateTime;
    a.HighPart = ftA.dwHighDateTime;

    b.LowPart = ftB.dwLowDateTime;
    b.HighPart = ftB.dwHighDateTime;

    return a.QuadPart - b.QuadPart;
}

bool CpuUsage::EnoughTimePassed()
{
    static const unsigned minElapsedMS = 250;   // milliseconds
    return Utils::TimeElapsed(lastRun_) > minElapsedMS;
}

}

#endif
