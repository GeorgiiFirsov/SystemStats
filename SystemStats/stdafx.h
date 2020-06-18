#pragma once

// Windows headers
#include <afxwin.h>
#include <afxcmn.h>
#include <Windows.h>
#include <tlhelp32.h>

// ATL headers
#include <atltypes.h>

// Application name
constexpr auto szApplicationName = _T("System Statistics");

// Unique ID generator
#define UNIQUE_ID (__COUNTER__ + 1)
