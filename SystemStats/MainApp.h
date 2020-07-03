#pragma once

// Project headers
#include "stdafx.h"
#include "DlgSave.h"
#include "DlgProcessInfo.h"
#include "Exception.h"
#include "ProcessesView.h"
#include "resource.h"
#include "i18n.h"

// Windows headers
#include <atlsync.h>

// STL headers
#include <atomic>


/* Useful macro declarations and global definitions */

// Macro to more easily distinguish message handlers
// between all other functions
#define MSG_HANDLER __stdcall

// Default update timeout (in seconds)
constexpr DWORD g_dwDefaultTimeout = 2;

// Maximum wait timeout for stopping scheduler (in seconds)
constexpr DWORD g_dwSchedulerWaitTimeout = 5;

// Window size
constexpr int g_iWndWidth = 1000;
constexpr int g_iWndHeight = 500;

// List view position
constexpr int g_iLVOffsetTop  = 28;
constexpr int g_iLVOffsetLeft = 0;

// Button sizes
constexpr int g_iBtnWidth  = 72;
constexpr int g_iBtnHeight = 22;

#define BUTTON_SIZE SIZE{ g_iBtnWidth, g_iBtnHeight }

// Snapshot button size, position and ID
const     UINT g_nSnapBtnID         = UNIQUE_ID;
constexpr int  g_iSnapBtnOffsetLeft = 5;
constexpr int  g_iSnapBtnOffsetTop  = 2;

// Save to file button size, position and ID
const     UINT g_nSaveBtnID         = UNIQUE_ID;
constexpr int  g_iSaveBtnOffsetLeft = 2 * g_iSnapBtnOffsetLeft + g_iBtnWidth;
constexpr int  g_iSaveBtnOffsetTop  = g_iSnapBtnOffsetTop;

// View default columns (std::initializer_list is fine here)
const auto g_columns = {
    system_stats::i18n::LoadUIString(IDS_EXECUTABLE),
    system_stats::i18n::LoadUIString(IDS_PID),
    system_stats::i18n::LoadUIString(IDS_PPID),
    system_stats::i18n::LoadUIString(IDS_THREADS)
};


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

    private:
        LRESULT MSG_HANDLER OnTimedUpdate(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam);
        LRESULT MSG_HANDLER OnDestroy(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam);
        LRESULT MSG_HANDLER OnSizing(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam);
        LRESULT MSG_HANDLER OnCommand(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam);
        LRESULT MSG_HANDLER OnSnapBtnPressed();
        LRESULT MSG_HANDLER OnSaveBtnPressed();
        LRESULT MSG_HANDLER OnViewItemDblClicked(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam);

    private:
        CMainApp()
            : m_WndClass({ sizeof(decltype(m_WndClass)) })
            , m_hInstance(nullptr)
            , m_hWnd(nullptr)
            , m_hSchedulerThread(nullptr)
            , m_hSchedulerWantStop(ATL::CEvent(TRUE, FALSE))
            , m_dwTimeOut(g_dwDefaultTimeout)
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

        // Update timeout value and timeout combo-box
        DWORD            m_dwTimeOut;

        // Various auxilary flags
        std::atomic_bool m_bInitialized;
    };

    // Macro used to obtain an instance of application
    #define Application system_stats::CMainApp::Instance()

} // namespace system_stats
