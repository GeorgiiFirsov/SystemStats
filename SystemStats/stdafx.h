#pragma once

// Window styles
#pragma comment(\
    linker, \
    "\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define NOMINMAX

// Windows headers
#include <afxwin.h>
#include <afxcmn.h>
#include <Windows.h>
#include <tlhelp32.h>

// ATL headers
#include <atltypes.h>

// Application name
constexpr auto szApplicationNameW = L"System Statistics";
constexpr auto szApplicationNameA = "System Statistics";

#ifdef UNICODE
#   define szApplicationName szApplicationNameW
#else
#   define szApplicationName szApplicationNameA
#endif // UNICODE


// Unique ID generator
#define UNIQUE_ID (__COUNTER__ + 1)


/* Custom messages declaration */

// Message used for update data about processes
#define WM_TIMEDUPDATE      (WM_USER + 1)
#define WM_VIEWITEMDBLCLICK (WM_USER + 2)
