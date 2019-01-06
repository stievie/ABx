/****************************** Module Header ******************************\
* Module Name:  CppWindowsService.cpp
* Project:      CppWindowsService
* Copyright (c) Microsoft Corporation.
*
* The file defines the entry point of the application. According to the
* arguments in the command line, the function installs or uninstalls or
* starts the service by calling into different routines.
*
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
* All other rights reserved.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#include "stdafx.h"

#if defined(_WIN32) && defined(WIN_SERVICE)

#pragma region Includes
#include <stdio.h>
#include <windows.h>
#include "AppService.h"
#include "ServiceInstaller.h"
#include "WinServiceBase.h"
#pragma endregion

//
//  FUNCTION: wmain(int, wchar_t *[])
//
//  PURPOSE: entrypoint for the application.
//
//  PARAMETERS:
//    argc - number of command line arguments
//    argv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    wmain() either performs the command line task, or run the service.
//
int wmain(int argc, wchar_t* argv[])
{
    if ((argc > 1) && ((*argv[1] == L'-' || (*argv[1] == L'/'))))
    {
        if (_wcsicmp(L"install", argv[1] + 1) == 0)
        {
            // Install the service when the command is
            // "-install" or "/install".
            System::InstallService(
                SERVICE_NAME,               // Name of service
                SERVICE_DISPLAY_NAME,       // Name to display
                SERVICE_START_TYPE,         // Service start type
                SERVICE_DEPENDENCIES,       // Dependencies
                SERVICE_ACCOUNT,            // Service running account
                SERVICE_PASSWORD            // Password of the account
                );
        }
        else if (_wcsicmp(L"remove", argv[1] + 1) == 0)
        {
            // Uninstall the service when the command is
            // "-remove" or "/remove".
            System::UninstallService(SERVICE_NAME);
        }
    }
    else
    {
        wprintf(L"Parameters:\n");
        wprintf(L" -install  to install the service.\n");
        wprintf(L" -remove   to remove the service.\n");
        AppService service;
        if (!System::WinServiceBase::Run(service))
        {
            wprintf(L"Service failed to run w/err 0x%08lx\n", GetLastError());
        }

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#endif
