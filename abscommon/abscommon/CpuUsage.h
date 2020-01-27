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

#pragma once

#include <stdint.h>
#if defined(AB_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif defined (AB_UNIX)
#include <ctime>
#endif

namespace System {

class CpuUsage
{
public:
    CpuUsage();

    /// Returns CPU usage of the calling process in percent, i.e. a value between 0..100.
    unsigned GetUsage();
private:
    unsigned cpuUsage_{ 0 };
    int64_t lastRun_{ 0 };
#if defined(AB_WINDOWS)
    ULONGLONG SubtractTimes(const FILETIME& ftA, const FILETIME& ftB);

    //system total times
    FILETIME prevSysKernel_;
    FILETIME prevSysUser_;

    //process times
    FILETIME prevProcKernel_;
    FILETIME prevProcUser_;

    volatile LONG runCount_;
#elif defined(AB_UNIX)
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
