//
// Copyright (c) 2008-2018 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "stdafx.h"

#if defined(_MSC_VER)

#include "StringUtils.h"
#include <cstdio>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <windows.h>
#pragma warning(push)
#pragma warning(disable: 4091)
#include <dbghelp.h>
#pragma warning(pop)
#include "Logger.h"
#include "MiniDump.h"

namespace System {

LONG UnhandledHandler(struct _EXCEPTION_POINTERS* apExceptionInfo)
{
    char fn[MAX_PATH];
    GetModuleFileNameA(nullptr, fn, MAX_PATH);
    std::string filename(fn);
    size_t pos = filename.find_last_of("\\/");
    filename = filename.substr(pos + 1, std::string::npos);
    return WriteMiniDump(filename.c_str(), apExceptionInfo);
}

int WriteMiniDump(const char* applicationName, EXCEPTION_POINTERS* exceptionPointers)
{
    static bool miniDumpWritten = false;

    // In case of recursive or repeating exceptions, only write the dump once
    /// \todo This function should not allocate any dynamic memory
    if (miniDumpWritten)
        return EXCEPTION_EXECUTE_HANDLER;

    miniDumpWritten = true;

    MINIDUMP_EXCEPTION_INFORMATION info;
    info.ThreadId = GetCurrentThreadId();
    info.ExceptionPointers = (EXCEPTION_POINTERS*)exceptionPointers;
    info.ClientPointers = TRUE;

    std::chrono::time_point<std::chrono::system_clock> time_point;
    time_point = std::chrono::system_clock::now();
    std::time_t ttp = std::chrono::system_clock::to_time_t(time_point);
    tm p;
    localtime_s(&p, &ttp);
    char dateTime[50];
    strftime(dateTime, 50, "%Y-%m-%d-%H-%M-%S", (const tm*)&p);
    std::string dateTimeStr = std::string(dateTime);

    char curDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, curDir);
    std::string miniDumpDir = std::string(curDir) + std::string("\\dumps");

    std::string fileName = miniDumpDir + "\\" + dateTimeStr + "_" + std::string(applicationName);
    std::string miniDumpName = fileName + ".dmp";

    CreateDirectoryA(miniDumpDir.c_str(), nullptr);
    HANDLE file = CreateFileA(miniDumpName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ,
        nullptr, CREATE_ALWAYS, 0, nullptr);

    BOOL success = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
        file, MiniDumpWithDataSegs, &info, nullptr, nullptr);
    CloseHandle(file);

    if (success)
    {
        // Write stack trace to text file
        std::string stackTraceName = fileName + ".txt";
        WriteStackTrace(stackTraceName, exceptionPointers);
    }

    if (success)
        LOG_ERROR << "An unexpected error occurred. A minidump was generated to " + miniDumpName;
    else
        LOG_ERROR << "An unexpected error occurred. Could not write minidump.";

    return EXCEPTION_EXECUTE_HANDLER;
}

// Prints stack trace based on context record
void WriteStackTrace(const std::string& fileName, EXCEPTION_POINTERS* exceptionPointers)
{
    BOOL    result;
    HANDLE  process;
    HANDLE  thread;
    HMODULE hModule;

    STACKFRAME64        stack;
    ULONG               frame;
    DWORD64             displacement;

    DWORD disp;
    IMAGEHLP_LINE64 *line;

    std::ofstream out(fileName);
    out << "*** Exception 0x" << std::hex << exceptionPointers->ExceptionRecord->ExceptionCode <<
        " at 0x" << std::hex << exceptionPointers->ExceptionRecord->ExceptionAddress <<
        " occurred ***" << std::endl << std::endl;

    CONTEXT* ctx = exceptionPointers->ContextRecord;
    char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
    char module[MAX_PACKAGE_NAME];
    PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;

    memset(&stack, 0, sizeof(STACKFRAME64));

    process = GetCurrentProcess();
    thread = GetCurrentThread();
    displacement = 0;
#if !defined(_M_AMD64)
    stack.AddrPC.Offset = (*ctx).Eip;
    stack.AddrPC.Mode = AddrModeFlat;
    stack.AddrStack.Offset = (*ctx).Esp;
    stack.AddrStack.Mode = AddrModeFlat;
    stack.AddrFrame.Offset = (*ctx).Ebp;
    stack.AddrFrame.Mode = AddrModeFlat;
#endif

    SymInitialize(process, NULL, TRUE); //load symbols

    for (frame = 0; ; frame++)
    {
        //get next call from stack
        result = StackWalk64
        (
#if defined(_M_AMD64)
            IMAGE_FILE_MACHINE_AMD64
#else
            IMAGE_FILE_MACHINE_I386
#endif
            ,
            process,
            thread,
            &stack,
            ctx,
            NULL,
            SymFunctionTableAccess64,
            SymGetModuleBase64,
            NULL
        );

        if (!result)
            break;

        //get symbol name for address
        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        pSymbol->MaxNameLen = MAX_SYM_NAME;
        SymFromAddr(process, (ULONG64)stack.AddrPC.Offset, &displacement, pSymbol);

        line = (IMAGEHLP_LINE64 *)malloc(sizeof(IMAGEHLP_LINE64));
        line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        // try to get line
        if (SymGetLineFromAddr64(process, stack.AddrPC.Offset, &disp, line))
        {
            out << "\tat " << pSymbol->Name << " in " << line->FileName <<
                ": line: " << std::dec << line->LineNumber << ": address: 0x" << std::hex << pSymbol->Address <<
                std::endl;
        }
        else
        {
            //failed to get line
            out << "\tat " << pSymbol->Name << ", address: 0x" << std::hex << pSymbol->Address <<
                std::endl;
            hModule = NULL;
            lstrcpyA(module, "");
            GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                (LPCTSTR)(stack.AddrPC.Offset), &hModule);

            //at least print module name
            if (hModule != NULL)
                GetModuleFileNameA(hModule, module, MAX_PATH);

            out << "in " << module << std::endl;
        }

        free(line);
        line = NULL;
    }
}

#pragma comment(lib, "dbghelp.lib")

}

#endif
