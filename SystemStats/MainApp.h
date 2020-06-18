#pragma once

// Project headers
#include "stdafx.h"
#include "DlgSave.h"
#include "Exception.h"
#include "ProcessesView.h"

// Windows headers
#include <atlsync.h>

// STL headers
#include <atomic>


/* Custom messages declaration */

// Message used for update data about processes
#define WM_TIMEDUPDATE (WM_USER + 1)


/* Useful macro declarations and global definitions */

// Macro to more easily distinguish message handlers
// between all other functions
#define MSG_HANDLER __stdcall

// Update timeout (in seconds)
constexpr size_t ullTimeout = 2;

// Maximum wait timeout for stopping scheduler (in seconds)
constexpr size_t ullSchedulerWaitTimeout = 5;

// Minimal window size
constexpr int iWndMinWidth  = 500;
constexpr int iWndMinHeight = 300;

// List view position
constexpr int iLVOffsetTop  = 28;
constexpr int iLVOffsetLeft = 0;

// Button sizes
constexpr int iBtnWidth  = 72;
constexpr int iBtnHeight = 22;

#define BUTTON_SIZE SIZE{ iBtnWidth, iBtnHeight }

// Snapshot button size, position and ID
const     UINT nSnapBtnID         = UNIQUE_ID;
constexpr int  iSnapBtnOffsetLeft = 5;
constexpr int  iSnapBtnOffsetTop  = 2;
constexpr auto szSnapBtnText      = L"Snapshot";

// Save to file button size, position and ID
const     UINT nSaveBtnID         = UNIQUE_ID;
constexpr int  iSaveBtnOffsetLeft = 2 * iSnapBtnOffsetLeft + iBtnWidth;
constexpr int  iSaveBtnOffsetTop  = iSnapBtnOffsetTop;
constexpr auto szSaveBtnText      = L"Dump";

// Default columns (std::initializer_list is fine here)
const auto g_columns = { L"Executable", L"PID", L"Parent PID", L"Thread count" };


namespace system_stats
{

    // Main application class. Implemented as singleton.
    // It implements a window procedure and all message handlers.
    class CMainApp
    {
    public:
        // Gives access to app instance
        static CMainApp& Instance();

        // Window procedure
        static LRESULT WINAPI WndProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);

        // Creates window class and registers it
        void InitializeInstance(_In_ HINSTANCE hInstance, _In_ LPCTSTR pszClassName);

        // Creates main window and shows it
        void CreateAndShowWindow(_In_ const CRect& Rect, _In_ int nShowCmd);

    private:
        DWORD _UpdateInfo();

        void _RunScheduler();

        void _StopScheduler();

        bool _IsSchedulerRunning();

        static unsigned int __stdcall _SchedulerProcedure(_In_ void* pThis) noexcept;

    public:
        LRESULT MSG_HANDLER OnTimedUpdate(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam);
        LRESULT MSG_HANDLER OnSize(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam);
        LRESULT MSG_HANDLER OnSizing(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam);
        LRESULT MSG_HANDLER OnCommand(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam);
        LRESULT MSG_HANDLER OnSnapBtnPressed();
        LRESULT MSG_HANDLER OnSaveBtnPressed();

    private:
        CMainApp()
            : m_WndClass({ 0 })
            , m_hInstance(nullptr)
            , m_hWnd(nullptr)
            , m_hSchedulerThread(nullptr)
            , m_hSchedulerWantStop(ATL::CEvent(TRUE, FALSE))
            , m_bInitialized(false)
        { };

        ~CMainApp() { _StopScheduler(); }

        CMainApp(const CMainApp&) = delete;
        CMainApp& operator=(const CMainApp&) = delete;
        CMainApp(CMainApp&&) = delete;
        CMainApp& operator=(CMainApp&&) = delete;

    private:
        // Window class instance
        WNDCLASSEX       m_WndClass;

        // Current application instance
        HINSTANCE        m_hInstance;

        // Main window descriptor
        HWND             m_hWnd;

        // Background thread handle (timed scheduler)
        // and its "stop-token" for cancellation
        HANDLE           m_hSchedulerThread;
        ATL::CEvent      m_hSchedulerWantStop;

        // Graphical view of processes
        CProcessesView   m_View;

        // Snapshot button
        CButton          m_hSnapBtn;

        // Save button
        CButton          m_hSaveBtn;

        // Various auxilary flags
        std::atomic_bool m_bInitialized;
    };

    // Macro used to obtain an instance of application
    #define Application system_stats::CMainApp::Instance()

} // namespace system_stats
