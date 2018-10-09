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
    return IsDebuggerPresent() || IsRmtDbgPresent() || (CheckHardwareBreakpoints() > 0) || CheckDbgPresentCloseHandle();
}

}

#endif
