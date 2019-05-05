#include "stdafx.h"

#if defined(AB_WINDOWS)

#include "CpuUsage.h"

namespace System {

CpuUsage::CpuUsage() :
    cpuUsage_(-1),
    lastRun_(0),
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
short CpuUsage::GetUsage()
{
    //create a local copy to protect against race conditions in setting the
    //member variable
    short nCpuCopy = cpuUsage_;
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
            {
                cpuUsage_ = (short)((100.0 * nTotalProc) / nTotalSys);
            }
        }

        prevSysKernel_ = ftSysKernel;
        prevSysUser_ = ftSysUser;
        prevProcKernel_ = ftProcKernel;
        prevProcUser_ = ftProcUser;

        lastRun_ = GetTickCount64();

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
    static const int minElapsedMS = 250;   // milliseconds

    ULONGLONG dwCurrentTickCount = GetTickCount64();
    return (dwCurrentTickCount - lastRun_) > minElapsedMS;
}

}

#endif