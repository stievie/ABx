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

#ifdef _WIN32

#include <Windows.h>

namespace AntiRE {

// CheckHardwareBreakpoints returns the number of hardware
// breakpoints detected and on failure it returns -1.
int CheckHardwareBreakpoints()
{
    unsigned int NumBps = 0;

    // This structure is key to the function and is the
    // medium for detection and removal
    CONTEXT ctx;
    ZeroMemory(&ctx, sizeof(CONTEXT));

    // The CONTEXT structure is an in/out parameter therefore we have
    // to set the flags so Get/SetThreadContext knows what to set or get.
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    // Get a handle to our thread
    HANDLE hThread = GetCurrentThread();

    // Get the registers
    if (GetThreadContext(hThread, &ctx) == 0)
        return -1;

    // Now we can check for hardware breakpoints, its not
    // necessary to check Dr6 and Dr7, however feel free to
    if (ctx.Dr0 != 0)
        ++NumBps;
    if (ctx.Dr1 != 0)
        ++NumBps;
    if (ctx.Dr2 != 0)
        ++NumBps;
    if (ctx.Dr3 != 0)
        ++NumBps;

    return NumBps;
}

// CheckCloseHandle will call CloseHandle on an invalid
// DWORD aligned value and if a debugger is running an exception
// will occur and the function will return true otherwise it'll
// return false
inline bool CheckDbgPresentCloseHandle()
{
    HANDLE Handle = (HANDLE)0x8000;
    __try
    {
        CloseHandle(Handle);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return true;
    }

    return false;
}

bool IsRmtDbgPresent()
{
    BOOL res = FALSE;
    if (CheckRemoteDebuggerPresent(GetCurrentProcess(), &res))
        return res ? true : false;
    return false;
}

bool IsDebuggee()
{
    return false;
//    return IsDebuggerPresent() || IsRmtDbgPresent() || (CheckHardwareBreakpoints() > 0) || CheckDbgPresentCloseHandle();
}

}

#endif
