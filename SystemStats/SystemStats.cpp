// Project headers
#include "stdafx.h"
#include "Exception.h"
#include "MainApp.h"


/* Global declarations */

// Global application instance
HINSTANCE g_hInstance;


// Application entry point
int CALLBACK wWinMain( 
    _In_     HINSTANCE hInstance, 
    _In_opt_ HINSTANCE hPrevInstance, 
    _In_     LPWSTR    lpCmdLine, 
    _In_     int       nShowCmd 
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    namespace exc = system_stats::exception;

    try
    {
        g_hInstance = hInstance;

        //
        // Initialize an application window - create window class
        // and register it
        // 

        Application.InitializeInstance(hInstance, szApplicationName);

        //
        // Create and show window
        // 

        CPoint TopLeft(CW_USEDEFAULT, CW_USEDEFAULT);
        CRect Rect(TopLeft, SIZE{g_iWndWidth, g_iWndHeight});

        Application.CreateAndShowWindow(Rect, nShowCmd);

        //
        // Run message loop
        // 

        MSG Msg;
        while (::GetMessage(&Msg, nullptr, 0, 0))
        {
            ::TranslateMessage(&Msg);
            ::DispatchMessage(&Msg);
        }

        return static_cast<int>(Msg.wParam);
    }
    catch(const exc::CWin32Error& error)
    {
        exc::DisplayErrorMessage(error);
        return error.Code();
    }
}